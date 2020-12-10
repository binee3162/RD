/*
  ==============================================================================

    DSPEffects.cpp
    Created: 9 Dec 2020 5:36:10pm
    Author:  Maximiliaan Vanackere & Ramadan Krasniqi

  ==============================================================================
*/
#include <math.h>
#include <JuceHeader.h>
#include "DSPEffects.h"
#pragma once




DSPEffects::DSPEffects() : 
    forwardFFT(fftOrder),
    inverseFFT(fftOrder),
    interPol(),
    theWindow(fftSize + 1, dsp::WindowingFunction<float>::hann)
{
    //setAudioChannels(1, 1);
}

DSPEffects::~DSPEffects()
{
    shutdownAudio();
}



void DSPEffects::preparePitchShift()
{
    mCircInBuffer.setSize(2, 1792); //size is frameSize + 3 * stepSize
    mCircOutBuffer.setSize(2, 4096);
    mInPointer = 0;
    mOutWritePointer = stepSize;
    mOutReadPointer = 0;
    stepCounter = 0;
}

void DSPEffects::pitchShiftInit(const AudioSourceChannelInfo& bufferToFill) //const float* inBuffer
{
    auto* device = deviceManager.getCurrentAudioDevice();
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    const int bufferLength = 1024;
    const int circInLength = mCircInBuffer.getNumSamples();
    const int circOutLength = mCircOutBuffer.getNumSamples();

    auto pitchShift = pitchSlider.getValue() / 100; //pitchShift ratio
    stepSizeOut = round(pow(2, pitchShift) * stepSize);


    for (auto channel = 0; channel < maxOutputChannels; ++channel)
    {
        if ((!activeOutputChannels[channel]) || maxInputChannels == 0)
        {
            bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
        }
        else
        {
            auto actualInputChannel = channel % maxInputChannels; // [1]

            if (!activeInputChannels[channel]) // [2]
            {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            }
            else // [3]
            {
                const float* bufferData = bufferToFill.buffer->getReadPointer(channel);
                auto* circIn = mCircInBuffer.getReadPointer(actualInputChannel);
                auto* circOut = mCircOutBuffer.getWritePointer(channel);
                auto* inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannel, bufferToFill.startSample); //get pointer to input sample
                auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample); //get pointer to output sample

                for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                {
                    mCircInBuffer.setSample(channel, mInPointer, inBuffer[sample]); //write input sample to circInBuffer
                    mInPointer++;
                    if (mInPointer >= circInLength) mInPointer = 0; //wrap around

                    float out = mCircOutBuffer.getSample(channel, mOutReadPointer);
                    mCircOutBuffer.setSample(channel, mOutReadPointer, 0); //set back to zero when sample has been read from output circular buffer
                    mOutReadPointer++;
                    if (mOutReadPointer >= circOutLength) mOutReadPointer = 0; //wrap around

                    out *= 0.25; //scaling for controlled volume
                    stepCounter++;
                    if (stepCounter >= stepSize)
                    {
                        shiftPitch(circIn, circOut, mInPointer, mOutWritePointer, channel, circInLength, circOutLength, pitchShift);

                        stepCounter = 0;
                        mOutWritePointer = (mOutWritePointer + stepSize) % circOutLength;

                    }
                    outBuffer[sample] = out;///
                }
            }
        }
    }
}

void DSPEffects::shiftPitch(const float* circIn, const float* circOut, int mInPointer, int mOutWritePointer, int channel, int circInLength, int circOutLength, float pitchShift)
{
    zeromem(fftData, sizeof(fftData));

    int mCircIndexIn = (mInPointer - fftSize + circInLength) % circInLength;

    float windbuf[1024] = { 0 };
    for (auto i = 0; i < fftSize; ++i)
    {
        windbuf[i] = mCircInBuffer.getSample(channel, mCircIndexIn);
        mCircIndexIn++;
        if (mCircIndexIn >= circInLength) mCircIndexIn = 0;
    }


    //apply window  to fft data
    theWindow.multiplyWithWindowingTable(windbuf, fftSize);
    memcpy(fftData, windbuf, sizeof(windbuf));
    //do FFT
    forwardFFT.performRealOnlyForwardTransform(fftData, true);


    //do processing phase vocoder
    float phasePart[fftSize];
    float magPart[fftSize];
    float deltaPhi[fftSize];
    float trueFreq[fftSize];
    for (auto j = 0; j < fftSize * 2; j = j + 2)
    {
        phasePart[j / 2] = atan(fftData[j + 1] / fftData[j]); //returns angle of imaginary number between -pi/2 and pi/2
        magPart[j / 2] = sqrt(pow(fftData[j + 1], 2) + pow(fftData[j], 2)); //calculate magnitude
        //calculate phase difference
        deltaPhi[j / 2] = phasePart[j / 2] - prevPhase[j / 2];
        prevPhase[j / 2] = phasePart[j / 2];
        deltaPhi[j / 2] = deltaPhi[j / 2] - (stepSize * 2 * MathConstants<float>::pi * (j / 2) / fftSize);
        //set in range -pi to pi
        deltaPhi[j / 2] = (fmod(deltaPhi[j / 2], (2 * MathConstants<float>::pi)) - MathConstants<float>::pi); //fmod returns floating point reaminder of division
        //true frequency
        trueFreq[j / 2] = 2 * MathConstants<float>::pi * (j / 2) / fftSize + deltaPhi[j / 2] / stepSize;
        phaseFinal[j / 2] = phaseFinal[j / 2] + stepSizeOut * trueFreq[j / 2];
        fftData[j] = magPart[j / 2] * cos(phaseFinal[j / 2]);
        fftData[j + 1] = magPart[j / 2] * sin(phaseFinal[j / 2]);
    }

    //do inverse FFT
    inverseFFT.performRealOnlyInverseTransform(fftData);

    float windbuf2[2048];
    for (int i = 0; i < fftSize; ++i)
    {
        windbuf2[i] = fftData[i];
    }
    //window and resample
    if (pitchShift != 0) theWindow.multiplyWithWindowingTable(windbuf2, pow(2, pitchShift) * frameSize);
    interPol.reset();
    if (pitchShift != 0) interPol.process(pow(2, pitchShift), windbuf2, windbuf, 1024);

    //copy output frame to output
    int mCircIndexOut = mOutWritePointer;
    //resample
    for (auto i = 0; i < fftSize; ++i)
    {
        mCircOutBuffer.addSample(channel, mCircIndexOut, windbuf[i]);
        mCircIndexOut++;
        if (mCircIndexOut >= circOutLength) mCircIndexOut = 0;
    }

}
/*
void DSPEffects::resized() 
{
    auto border = 4;

    auto area = getLocalBounds();

    pitchSlider.setBounds(20, 20, 140, 140);
    pitchLabel.setBounds(10, 10, 90, 20);
}*/

void DSPEffects::initAdaptiveFilter(unsigned int samplingRate, float lambda, float p)
{
    Fs = samplingRate;
    lambda = this->lambda;
    p = this->p;
}

void DSPEffects::setAdaptiveFilter(int N)
{ //FIR filter

    setFilterLength(N);

    for (unsigned int i = 0; i < filterLength; i++) {
        filterCoeff[i] = 0;
        sigma_yy[i][i] = 0.001;
        num[i] = 0;
        den[i] = 0;
    }
}

void DSPEffects::setFilterLength(int N)
{
    filterLength = N;
}

void DSPEffects::applyAdaptiveFilter(std::vector<float>& inputSequence) {

    // Estimate noise characteristics (first 200 ms)
    float temp = 0;
    std::vector<float> n(Fs / 5);
    for (unsigned int i = 0; i < Fs / 5; i++)
    {
        temp += inputSequence[i] * inputSequence[i];
        n.push_back(inputSequence[i]);
    }
    auto n_th = temp / (Fs / 5);

    // Select rest of buffered samples
    auto rest = std::vector<float>(inputSequence.begin() + (Fs / 5), inputSequence.end());

    // Amount of samples in a 10 ms frame
    float spf = Fs / 100;

    for (int i = 0; i < rest.size() / spf; i++)
    {

        auto temp = std::vector<float>(rest.begin() + i * spf, rest.begin() + i * spf + spf);

        // Compute power of 10 ms input frame
        float temp_avg_p = 0;
        for (unsigned int k = 0; k < temp.size(); k++)
        {
            temp_avg_p += temp[k] * temp[k];
        }
        temp_avg_p = temp_avg_p / temp.size();
        //std::cout << "Value temp_avg_p: " << temp_avg_p << std::endl;

        if (temp_avg_p > n_th)
        {

            // Frame is ACTIVE, adaptive filter is applied
            for (unsigned int j = 0; j < temp.size() - filterLength; j++)
            {

                auto x = std::vector<float>(temp.begin() + j, temp.begin() + filterLength + j);
                auto noise_frame = std::vector<float>(n.begin() + j, n.begin() + filterLength + j);

                updateFilterCoeff(x, noise_frame, lambda);
            }
        }
        else
        {

            // Frame is INACTIVE, noise threshold is updated
            n_th = (1 - p) * n_th + p * temp_avg_p;
        }
    }
}

void DSPEffects::updateFilterCoeff(std::vector<float>& x, std::vector<float>& n,
    float& lambda)
{
    auto des = x.end() - (*n.end()) * (*n.end());

    // Matrix multiplication
    for (unsigned int row = 0; row < filterLength; row++)
    {
        for (unsigned index = 0; index < filterLength; index++)
        {
            num[row] += (1 / lambda) * (sigma_yy[row][index] * x[index]);
            den[row] += 1 + (1 / lambda) * (x[index] * sigma_yy[row][index] * x[index]);
        }
    }

    // Compute Gain
    for (unsigned int i = 0; i < filterLength; i++)
    {
        gain[i] = num[i] / den[i];
    }

    // Update filter coeff
    for (unsigned int i = 0; i < filterLength; i++)
    {
        filterCoeff[i] = filterCoeff[i] + gain[i] * (*des - filterCoeff[i] * x[i]);
    }

    // Update auto corr matrix
    for (unsigned int row = 0; row < filterLength; row++)
    {
        for (unsigned int col = 0; col < filterLength; col++)
        {

            for (unsigned index = 0; index < filterLength; index++)
            {
                sigma_yy[row][col] = (1 / lambda) * sigma_yy[row][col] - (1 / lambda) * gain[index] * x[index] * sigma_yy[row][index];
            }
        }
    }
}

void DSPEffects::filterInputData(std::vector<float>& inputSequence)
{

}
