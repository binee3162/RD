/*
  ==============================================================================

    ScrollBar.h
    Created: 12 Nov 2020 8:28:24pm
    Author:  clayt

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class WaveScrollBar : public juce::AudioVisualiserComponent,
    public juce::AudioIODeviceCallback
{
public:
    std::vector<float> allBuffer;
    float inpuChannelDatas[2048];
    float* inputChannelDatas;
    int numInputChannel;
    float outputChannelDatas;
    int numOutputChannel;
    int numberOfSample;
    float* inputpointer;
    bool eq{ false };
    bool eq2{ false };
    juce::AudioBuffer <float> audioBuffer{ 1,1 };
    WaveScrollBar() : AudioVisualiserComponent(1) {
        setSamplesPerBlock(256);
        setBufferSize(1024);
    }

    void audioDeviceAboutToStart(juce::AudioIODevice*) override
    {
        clear();

    }
    void audioDeviceStopped() override
    {
        clear();
    }
    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels,
        int numberOfSamples) override
    {
        //inpuChannelDatas = **inputChannelData;

        numInputChannel = numInputChannels;
        outputChannelDatas = **outputChannelData;
        numOutputChannel = numOutputChannels;
        numberOfSample = numberOfSamples;

        auto* outBuffer = audioBuffer.getWritePointer(0, 0);

        for (int i = 0; i < numberOfSamples; ++i)
        {
            float inputSample = 0;

            for (int chan = 0; chan < numInputChannels; ++chan)
                if (const float* inputChannel = inputChannelData[chan])
                    inputSample += inputChannel[i];  // find the sum of all the channels

            inpuChannelDatas[i] = inputSample;
            //inputChannelDatas[i] = inputSample;
            //eq2 = inpuChannelDatas[i] == inputChannelDatas[i];
            //outBuffer[i] = inpuChannelDatas[i];
            allBuffer.push_back(inpuChannelDatas[i]);
            pushSample(&inputSample, 1);
        }

        inputpointer = &(inpuChannelDatas[0]);
        audioBuffer.setDataToReferTo(&inputpointer, numInputChannels, numberOfSamples);
        eq = (*inputpointer == inpuChannelDatas[0]);
        




        // We need to clear the output buffers before returning, in case they're full of junk..
        for (int j = 0; j < numOutputChannels; ++j)
            if (float* outputChannel = outputChannelData[j])
                juce::zeromem(outputChannel, (size_t)numberOfSamples * sizeof(float));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveScrollBar)

};