#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (800, 600);
    
    // Initialise audio engine with 0 inputs and 2 outputs
    audioEngine.setAudioChannels (0, 2);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::FontOptions (16.0f));
    g.setColour (juce::Colours::white);
    
    g.drawText ("DJ Edit Lab - Phase 1: Audio Engine Fundament", 
                getLocalBounds(), juce::Justification::centred, true);
    
    g.setColour (juce::Colours::green);
    g.drawText ("Audio thread is running (Check logs for status)", 
                getLocalBounds().translated(0, 40), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
