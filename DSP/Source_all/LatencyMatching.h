/*
  ==============================================================================

    LatencyMatching.h
    Created: 10 Dec 2020 9:52:49am
    Author:  Ramadan Krasniqi

  ==============================================================================
*/

#ifndef LATENCYMATCHING_H
#define LATENCYMATCHING_H

#include <JuceHeader.h>

#pragma once

class LatencyMatching
{
public:

    LatencyMatching();
    void matchSignals(const float* channel1, const float* channel2);
    float* getOutputBuffer() { return outputBuffer; }

    static constexpr auto fftOrder = 10;                    // [1]
    static constexpr auto fftSize = 1 << fftOrder;         // [2]

private:

    juce::dsp::FFT forwardFFT;
    juce::dsp::FFT forwardFFT2;
    juce::dsp::FFT inverseFFT;

    float outputBuffer[fftSize] = { 0 };
};

#endif
