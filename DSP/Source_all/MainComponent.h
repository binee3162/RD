#pragma once

#include <JuceHeader.h>
#include "AudioDevice.h"
#include "DSPEffects.h"
#include "LatencyMatching.h"

using namespace juce;
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  :  public juce::AudioAppComponent,
                        public juce::ChangeListener,
                        public Slider::Listener,
                        public Button::Listener
                        

{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void sliderValueChanged(Slider* slider) override
    {
        if(slider==&inputSlider){
            ratio = inputSlider.getValue();
            //logMessage("slider value has been changed to " + std::to_string(ratio));
        }
    };
    void updateToggleState(juce::Button* button, juce::String name);
    void buttonClicked(juce::Button* button) override // [2]
    {

    };
private:
    //==============================================================================
    // Your private member variables go here...
    
    void infoButtonClicked();
    void logMessage(const juce::String& m)
    {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret(m + juce::newLine);
        diagnosticsBox.insertTextAtCaret(juce::newLine);
    }
    Label bufferlabel;
    AudioDeviceManager otherDeviceManager;
    std::unique_ptr <AudioDeviceSelectorComponent> audioSettings;     

    AudioDevice subAudioDevice;

    //std::shared_ptr<DSPEffects> dspObj;
    std::shared_ptr<LatencyMatching> latency;

    juce::TextEditor diagnosticsBox;
    int position = 0;
    int readMatched = 0;
    int bufferCount = 0;
    WaveScrollBar w;
    WaveScrollBar w2;

    WaveScrollBar w3;

    AudioBuffer<float> channelBuffer; 

    Slider inputSlider;
    Slider pitchSlider;
    Label pitchLabel;
    ToggleButton filterButton{ "Adaptive Filter On" };

    bool adaptiveFilterOn = false;

    DSPEffects dspObj;

    TextButton infoButton;
    float ratio = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
