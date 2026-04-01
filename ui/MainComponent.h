#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../engine/AudioEngine.h"

//==============================================================================
/**
    Main UI — Phase 2: Audio Loading & Playback.

    Provides:
    - "Load File" button  → opens native file chooser, loads WAV/MP3
    - "Play / Stop" button → starts/stops AudioTransportSource
    - Status label         → shows filename + playback state
*/
class MainComponent : public juce::Component,
                      public juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Timer callback — polls playhead from audio thread for UI updates
    void timerCallback() override;

    //==============================================================================
    // UI helpers
    void loadFileButtonClicked();
    void playStopButtonClicked();
    void updatePlayStopButton();

    //==============================================================================
    AudioEngine audioEngine;

    juce::TextButton loadFileButton  { "Load File" };
    juce::TextButton playStopButton  { "Play" };
    juce::Label      statusLabel;
    juce::Label      positionLabel;

    // Owned here; shared_ptr not needed — chooser outlives callback on message thread
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::String loadedFileName;

    //==============================================================================
    static constexpr int kUiTimerHz = 30;   // 30 fps UI refresh

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
