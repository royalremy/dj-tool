#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../engine/AudioEngine.h"
#include "TimelineComponent.h"

//==============================================================================
/**
    Main UI — Phase 3: Timing & Quantization.

    Provides:
    - "Load File" button  → opens native file chooser, loads WAV/MP3
    - "Play" button       → starts quantized playback (next bar boundary)
    - "Stop" button       → stops playback immediately
    - BPM slider          → adjusts master clock BPM [60–200]
    - Status label        → shows filename + BPM
    - Position label      → shows mm:ss + Bar:Beat position
*/
class MainComponent : public juce::Component,
                      public juce::Timer
{
public:
    //==========================================================================
    MainComponent();
    ~MainComponent() override;

    //==========================================================================
    void paint  (juce::Graphics& g) override;
    void resized() override;

private:
    //==========================================================================
    // Timer — polls audio engine for UI updates (30 fps)
    void timerCallback() override;

    //==========================================================================
    // UI helpers
    void loadFileButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void bpmSliderChanged();
    void loop4ButtonClicked();
    void loop8ButtonClicked();
    void clearLoopButtonClicked();

    //==========================================================================
    AudioEngine audioEngine;

    juce::TextButton loadFileButton { "Load File" };
    juce::TextButton playButton     { "Play (quantized)" };
    juce::TextButton stopButton     { "Stop" };

    juce::TextButton loop4Button    { "Loop 4 Bars" };
    juce::TextButton loop8Button    { "Loop 8 Bars" };
    juce::TextButton clearLoopButton{ "Clear Loop" };

    juce::Slider     bpmSlider;
    juce::Label      bpmLabel;

    juce::Label      statusLabel;
    juce::Label      positionLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::String loadedFileName;

    TimelineComponent timeline;

    //==========================================================================
    static constexpr int    kUiTimerHz  = 30;
    static constexpr double kBpmMin     = 60.0;
    static constexpr double kBpmMax     = 200.0;
    static constexpr double kBpmDefault = 120.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
