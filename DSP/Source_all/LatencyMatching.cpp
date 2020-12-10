/*
  ==============================================================================

    LatencyMatching.cpp
    Created: 10 Dec 2020 9:52:49am
    Author:  Ramadan Krasniqi

  ==============================================================================
*/

#include "LatencyMatching.h"


LatencyMatching::LatencyMatching() : forwardFFT(fftOrder), forwardFFT2(fftOrder), inverseFFT(fftOrder)
{

}

void LatencyMatching::matchSignals(const float* channel1, const float* channel2)
{
    float fftChannel1[2 * fftSize] = { 0 };
    float fftChannel2[2 * fftSize] = { 0 };

    memcpy(fftChannel1, channel1, sizeof(channel1));
    memcpy(fftChannel2, channel2, sizeof(channel2));

    forwardFFT.performRealOnlyForwardTransform(fftChannel1, true);
    forwardFFT2.performRealOnlyForwardTransform(fftChannel2, true);

    // Compute complex conjugate of signal from second channel
    for (unsigned int i = 0; i < 2 * fftSize; i++)
    {
        if (i % 2 == 1) // Only invert the imaginary part
        {
            fftChannel2[i] = -fftChannel2[i];
        }
    }

    // Compute cross correlation in both channels
    float crosscorr[2 * fftSize] = { 0 };
    for (unsigned int i = 0; i < 2 * fftSize; i++)
    {
        crosscorr[i] = fftChannel1[i] * fftChannel2[i];
    }

    // Compute inverse FFT for cross correlation array
    inverseFFT.performRealOnlyInverseTransform(crosscorr);

    // Find index of maximum value in array containing cross correlation function
    int indexMaxElement = 0;
    float temp = crosscorr[0];
    for (unsigned int i = 0; i < 2 * fftSize; i++)
    {
        if (temp < crosscorr[i])
        {
            temp = crosscorr[i];
            indexMaxElement = i;
        }
    }

    float outputBuffer[fftSize] = { 0 };

    for (unsigned int i = 0; i < fftSize - indexMaxElement; i++)
    {
        if (i < fftSize - indexMaxElement)
            outputBuffer[i] = channel1[i] + channel2[i + indexMaxElement];
        else
            outputBuffer[i] = channel1[i];
    }
}
