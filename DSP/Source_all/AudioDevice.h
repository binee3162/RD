/*
  ==============================================================================

    AudioDevice.h
    Created: 21 Nov 2020 8:43:30pm
    Author:  clayt

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "WaveScrollBar.h"

using namespace juce;
//==============================================================================
/*
*/
class AudioDevice  : public juce::AudioAppComponent
{
public:
    AudioDevice();
    ~AudioDevice() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    AudioDeviceManager subDeviceManager;
private:
    void logMessage(const juce::String& m)
    {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret(m + juce::newLine);
    }
    std::unique_ptr <AudioDeviceSelectorComponent> subaudioSettings;
    Label bufferlabel;
    juce::TextEditor diagnosticsBox;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDevice)
};
