/*
  ==============================================================================

    DSPEffects.h
    Created: 9 Dec 2020 5:39:56pm
    Author:  Maximiliaan Vanackere & Ramadan Krasniqi
    Copyright © 2020 JUCE. All rights reserved.

  ==============================================================================
*/

#ifndef DSPEffects_h
#define DSPEffects_h

#include <stdio.h>
#include <math.h>
#include <memory>
#include <numeric>
#include <iostream>
#include <vector>
#include <JuceHeader.h>
#include "MainComponent.h"

#pragma once

#define _USE_MATH_DEFINES


class DSPEffects : public AudioAppComponent
{
public:
    DSPEffects();
    ~DSPEffects() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {}
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {}
    void releaseResources() override {}

    // Pitch shifting methods
    void preparePitchShift(); //override;
    void pitchShiftInit(const AudioSourceChannelInfo& bufferToFill);
    void shiftPitch(const float* circIn, const float* circOut,
        int mInPointer, int mOutWritePointer, int channel,
        int circInLength, int circOutLength, float pitchShift);
    //void resized() override;

    // Adaptive filtering methods
    void initAdaptiveFilter(unsigned int samplingRate, float lambda, float p);
    inline void setAdaptiveFilter(int N);
    void setFilterLength(int N);
    inline float* getFilterCoeff() { return filterCoeff; }
    inline float getFilterLenght() { return filterLength; }
    void applyAdaptiveFilter(std::vector<float>& inputSequence);
    void updateFilterCoeff(std::vector<float>& x, std::vector<float>& n, float& lambda);
    void filterInputData(std::vector<float>& inputSequence);

    static constexpr auto fftOrder = 10;
    static constexpr auto fftSize = 1 << fftOrder; //1024


private:
    Random random;
    Slider pitchSlider;
    Label pitchLabel;
    AudioBuffer<float> mCircInBuffer;
    AudioBuffer<float> mCircOutBuffer;

    int mInPointer;
    int mOutWritePointer;
    int mOutReadPointer;

    int stepCounter;

    dsp::WindowingFunction<float> theWindow;

    dsp::FFT forwardFFT;
    dsp::FFT inverseFFT;

    Interpolators::WindowedSinc interPol;

    float fftData[2 * fftSize] = { 0 };
    float prevPhase[fftSize] = { 0 };
    float phaseFinal[fftSize] = { 0 };

    int stepSize{ 256 };
    int frameSize{ 1024 };

    int stepSizeOut;
    int frameSizeOut;

    // Adaptive filtering variables
    unsigned int filterLength;
    unsigned int Fs;
    float lambda;
    float p;
    float filterCoeff[100];
    float sigma_yy[100][100];

    float num[100];
    float den[100];
    float gain[100];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DSPEffects)

};

#endif /* DSPEffects_h */
