/*
  ==============================================================================

    AudioDevice.cpp
    Created: 21 Nov 2020 8:43:30pm
    Author:  clayt

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AudioDevice.h"

//==============================================================================
AudioDevice::AudioDevice()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    subDeviceManager.initialise(1, 1, nullptr, true);
    subaudioSettings.reset(new AudioDeviceSelectorComponent(subDeviceManager, 0, 2, 0, 2, true, true, false, false));
    addAndMakeVisible(subaudioSettings.get());

    addAndMakeVisible(bufferlabel);
    bufferlabel.setText("sub default:", dontSendNotification);
    bufferlabel.attachToComponent(subaudioSettings.get(), true);

    addAndMakeVisible(diagnosticsBox);

    logMessage("Sub Default");


}

AudioDevice::~AudioDevice()
{
}
void AudioDevice:: prepareToPlay(int samplesPerBlockExpected, double sampleRate) {} ;
void AudioDevice:: getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    logMessage("getNextAudioBlock");
    //subbufferlabel.setText("sub getNextAudioBlock:", dontSendNotification);
};
void AudioDevice:: releaseResources() {};
void AudioDevice::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    //g.setColour (juce::Colours::grey);
    //g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    //g.setColour (juce::Colours::white);
    //g.setFont (14.0f);
    //g.drawText ("AudioDevice", getLocalBounds(),
    //            juce::Justification::centred, true);   // draw some placeholder text
}

void AudioDevice::resized()
{
    subaudioSettings->setBounds(0, 0, 400 - 20, 100);
    //diagnosticsBox.setBounds(0, 600, 400 - 20, 300);
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
