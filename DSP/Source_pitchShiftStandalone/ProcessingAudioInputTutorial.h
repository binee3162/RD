/******************************************************************************
* Pitch shift JUCE
* Created by Maximiliaan Vanackere

*******************************************************************************/

#include<math.h>
#pragma once

#define _USE_MATH_DEFINES

//==============================================================================
class MainContentComponent   : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainContentComponent()
        : forwardFFT(fftOrder),
        inverseFFT(fftOrder),
        interPol(),
        theWindow(fftSize + 1, juce::dsp::WindowingFunction<float>::hann)
    {
        pitchSlider.setRange (-40, 40, 1);
        pitchSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        pitchSlider.setTextValueSuffix("%");
        pitchSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 20);
        pitchLabel.setText ("Pitch shift", juce::dontSendNotification);

        addAndMakeVisible (pitchSlider);
        addAndMakeVisible (pitchLabel);
        pitchLabel.setText("Pitch shift", juce::dontSendNotification);
        pitchLabel.attachToComponent(&pitchSlider, true);

        setSize (400, 400);
        setAudioChannels (1, 1);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay (int, double) override 
    {
        mCircInBuffer.setSize(2, 1792); //size is frameSize + 3 * stepSize
        mCircOutBuffer.setSize(2, 4096);
        mInPointer = 0;
        mOutWritePointer = stepSize;
        mOutReadPointer = 0;
        stepCounter = 0;
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* device = deviceManager.getCurrentAudioDevice();
        auto activeInputChannels  = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();
        auto maxInputChannels  = activeInputChannels .getHighestBit() + 1;
        auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

        const int bufferLength = bufferToFill.buffer->getNumSamples();
        const int circInLength = mCircInBuffer.getNumSamples();
        const int circOutLength = mCircOutBuffer.getNumSamples();

        auto pitchShift = pitchSlider.getValue() / 100; //pitchShift ratio
        stepSizeOut = round(pow(2, pitchShift) * stepSize); 
        
        

        for (auto channel = 0; channel < maxOutputChannels; ++channel)
        {
            if ((! activeOutputChannels[channel]) || maxInputChannels == 0)
            {
                bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
            }
            else
            {
                auto actualInputChannel = channel % maxInputChannels; // [1]

                if (! activeInputChannels[channel]) // [2]
                {
                    bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
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
                        if(mInPointer >= circInLength) mInPointer = 0; //wrap around

                        float out = mCircOutBuffer.getSample(channel, mOutReadPointer);
                        mCircOutBuffer.setSample(channel, mOutReadPointer, 0); //set back to zero when sample has been read from output circular buffer
                        mOutReadPointer++;
                        if (mOutReadPointer >= circOutLength) mOutReadPointer = 0; //wrap around

                        //out *= 0.6; //scaling for controlled volume
                        stepCounter++;
                        if (stepCounter >= stepSize)
                        {
                            interPol.reset();
                            shiftPitch(circIn, circOut, mInPointer, mOutWritePointer, channel, circInLength, circOutLength, pitchShift);
                            
                            stepCounter = 0;
                            mOutWritePointer = (mOutWritePointer + stepSize) % circOutLength;
                            
                        }
                        outBuffer[sample] = out;
                    }
                }
            }
        }
    }

    void shiftPitch(const float* circIn, const float* circOut, int mInPointer, int mOutWritePointer, int channel, int circInLength, int circOutLength, float pitchShift)
    {
        juce::zeromem(fftData, sizeof(fftData));

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
            //remove expected phase difference
            deltaPhi[j / 2] = deltaPhi[j / 2] - (stepSize * 2 * juce::MathConstants<float>::pi * (j/2)/fftSize);
            //set in range -pi to pi
            deltaPhi[j / 2] = (fmod(deltaPhi[j / 2], (2 * juce::MathConstants<float>::pi)) - juce::MathConstants<float>::pi); //fmod returns floating point remainder of division
            //true frequency
            trueFreq[j / 2] = 2*juce::MathConstants<float>::pi*(j/2)/fftSize + deltaPhi[j/2]/stepSize;
            phaseFinal[j / 2] = phaseFinal[j / 2] + stepSizeOut * trueFreq[j / 2];
            fftData[j] =  magPart[j / 2] * cos(phaseFinal[j / 2]);
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
        if (pitchShift != 0) interPol.process(pow(2, pitchShift), windbuf2, windbuf, 1024);
        
        if (pitchShift != 0) theWindow.multiplyWithWindowingTable(windbuf, frameSize);
        
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

    void releaseResources() override {}

    void resized() override
    {
        auto border = 4;

        auto area = getLocalBounds();

        pitchSlider.setBounds(20, 20, 140, 140);
        pitchLabel.setBounds(10, 10, 90, 20);
    }

        static constexpr auto fftOrder = 10;
        static constexpr auto fftSize = 1 << fftOrder; //1024
        

private:
    juce::Random random;
    juce::Slider pitchSlider;
    juce::Label pitchLabel;
    juce::AudioBuffer<float> mCircInBuffer;
    juce::AudioBuffer<float> mCircOutBuffer;

    int mInPointer;
    int mOutWritePointer;
    int mOutReadPointer;

    int stepCounter;

    juce::dsp::WindowingFunction<float> theWindow;
   
    juce::dsp::FFT forwardFFT;
    juce::dsp::FFT inverseFFT;

    juce::Interpolators::WindowedSinc interPol;

    float fftData[2 * fftSize] = { 0 };
    float prevPhase[fftSize] = { 0 };
    float phaseFinal[fftSize] = { 0 };

    int stepSize { 256 };
    int frameSize { 1024 };

    int stepSizeOut;
    int frameSizeOut;
    bool nextFFTBlockReady = false;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
