#include "MainComponent.h"

using namespace std;
//==============================================================================
MainComponent::MainComponent() : AudioAppComponent(otherDeviceManager), infoButton("info")
{
    otherDeviceManager.initialise(2, 1, nullptr, true);
    audioSettings.reset(new AudioDeviceSelectorComponent(otherDeviceManager, 0, 2, 0, 2, true, true, false, false));
    addAndMakeVisible(audioSettings.get());
    // Make sure you set the size of the component after
    // you add any child components.
 
    
    addAndMakeVisible(bufferlabel);
    bufferlabel.setText("default", dontSendNotification); 
    bufferlabel.attachToComponent(audioSettings.get(), false);

    addAndMakeVisible(subAudioDevice);


    addAndMakeVisible(w);
    addAndMakeVisible(w2);

    addAndMakeVisible(w3);
    otherDeviceManager.addAudioCallback(&w);
    subAudioDevice.subDeviceManager.addAudioCallback(&w2);

    //otherDeviceManager.addAudioCallback(&w3);
    //subAudioDevice.subDeviceManager.addAudioCallback(&w3);

    addAndMakeVisible(diagnosticsBox);
    setSize(1200, 1200);
    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
    infoButton.onClick = [this] {  infoButtonClicked(); };
    addAndMakeVisible(&infoButton);

    addAndMakeVisible(inputSlider);
    inputSlider.setRange(-1, 1);
    inputSlider.addListener(this);
    //inputSlider.setTextValueSuffix(" left to right");
    //w.allBuffer contains all the data from device one;
    //w2.allBuffer contains all the data from device two;
    //In case you need more data samples than the buffer size.

    pitchSlider.setRange(-50, 50, 1);
    pitchSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    pitchSlider.setTextValueSuffix("%");
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    pitchLabel.setText("Pitch shift", juce::dontSendNotification);


    addAndMakeVisible(filterButton);
    filterButton.onClick = [this] { updateToggleState(&filterButton, "Filter");   };
    filterButton.setClickingTogglesState(true);

    addAndMakeVisible(pitchSlider);
    addAndMakeVisible(pitchLabel);
    pitchLabel.setText("Pitch shift", juce::dontSendNotification);
    pitchLabel.attachToComponent(&pitchSlider, true);

    dspObj = std::make_shared<DSPEffects>();
    latency = std::make_shared<LatencyMatching>();
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}
void MainComponent::updateToggleState(juce::Button* button, juce::String name)
{
    auto state = button->getToggleState();
    if (state == true) adaptiveFilterOn = true;
    else adaptiveFilterOn = false;
}

void MainComponent::infoButtonClicked() {
    //logMessage("Number of Samples" + std::to_string(w2.numberOfSample));

    logMessage("numInputChannel: "+std::to_string(w.numInputChannel));
    logMessage("numOutputChannel: "+std::to_string(w.numOutputChannel));
    logMessage("Number of Samples: " + std::to_string(w.numberOfSample));
    logMessage(std::to_string(w.audioBuffer.getNumChannels()));
    logMessage(std::to_string(w.audioBuffer.getNumSamples()));
    logMessage(std::to_string(w.allBuffer.size()));
    logMessage(std::to_string(w.eq2));
}


void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
}
//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    dspObj->preparePitchShift();
    readMatched = 0;
    channelBuffer.setSize(2, 1024);
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    //i++; 
    //logMessage(std::to_string(i));
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferlabel.setText("getNextAudioBlock", dontSendNotification);
    //auto* device = subAudioDevice.subDeviceManager.getCurrentAudioDevice();
    auto* device = otherDeviceManager.getCurrentAudioDevice();
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;
    
    
    auto level = 1.0;
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
                auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                auto* thisBuffer = bufferToFill.buffer->getReadPointer(channel, bufferToFill.startSample);
                
                if (!w2.numInputChannel) {
                    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                        outBuffer[sample] = 0;
                }
                else {
                    auto* inBuffer = w2.audioBuffer.getReadPointer(actualInputChannel, bufferToFill.startSample);
                    bufferCount++;
                    float *matchedSigBuffer;

                    if (bufferCount >= 1024)
                    {
                        bufferCount = 0; 
                        latency->matchSignals(inBuffer, thisBuffer);
                        matchedSigBuffer = latency->getOutputBuffer();
                    }
                    
                    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                    {
                        outBuffer[sample] = matchedSigBuffer[readMatched];
                        readMatched++;
                        if (readMatched >= 1024) readMatched = 0;
                    }
                    
                }
                
            }
        }
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    infoButton.setBounds(10, 10, 40, 40);
    audioSettings->setBounds(10, 100, 400 - 20, 100);
    subAudioDevice.setBounds(500, 100, 600, getHeight());
    w.setBounds(10, getHeight() / 2 + 10, getWidth()-10, 60);
    w2.setBounds(10, getHeight() / 2 + 100, getWidth()-20, 60);
    pitchSlider.setBounds(getWidth() - 200, 20, 140, 140);
    pitchLabel.setBounds(getWidth() - 200, 10, 90, 20);
    filterButton.setBounds(getWidth() - 200, 200, 140, 80);

    //w3.setBounds(10, getHeight() / 2 + 190, getWidth()-20, 60);
    inputSlider.setBounds(10, getHeight() / 2 + 190, getWidth() - 20, 60);
    diagnosticsBox.setBounds(10, getHeight() / 2 + 260, getWidth()-20, 300);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
