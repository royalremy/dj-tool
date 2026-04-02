#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // ── Status label ──────────────────────────────────────────────────────────
    statusLabel.setText ("No file loaded.", juce::dontSendNotification);
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    statusLabel.setFont (juce::FontOptions (14.0f));
    statusLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (statusLabel);

    // ── Position label ────────────────────────────────────────────────────────
    positionLabel.setText ("0:00 / 0:00  |  Bar 1  Beat 1", juce::dontSendNotification);
    positionLabel.setJustificationType (juce::Justification::centredRight);
    positionLabel.setFont (juce::FontOptions (14.0f));
    positionLabel.setColour (juce::Label::textColourId, juce::Colours::lightblue);
    addAndMakeVisible (positionLabel);

    // ── Load File button ──────────────────────────────────────────────────────
    loadFileButton.onClick = [this] { loadFileButtonClicked(); };
    loadFileButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2a4a));
    loadFileButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible (loadFileButton);

    // ── Play button (quantized) ───────────────────────────────────────────────
    playButton.setEnabled (false);
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a6b3c));
    playButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible (playButton);

    // ── Stop button ───────────────────────────────────────────────────────────
    stopButton.setEnabled (false);
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b1a1a));
    stopButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible (stopButton);

    // ── BPM slider ────────────────────────────────────────────────────────────
    bpmSlider.setRange (kBpmMin, kBpmMax, 0.5);
    bpmSlider.setValue (kBpmDefault, juce::dontSendNotification);
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 24);
    bpmSlider.onValueChange = [this] { bpmSliderChanged(); };
    bpmSlider.setColour (juce::Slider::thumbColourId,       juce::Colour (0xff4a90d9));
    bpmSlider.setColour (juce::Slider::trackColourId,       juce::Colour (0xff2a2a4a));
    bpmSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    bpmSlider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff1a1a2e));
    bpmSlider.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colour (0xff2a2a4a));
    addAndMakeVisible (bpmSlider);

    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    bpmLabel.setFont (juce::FontOptions (13.0f));
    bpmLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (bpmLabel);

    // Set initial BPM on the engine
    audioEngine.setBPM (kBpmDefault);

    // ── Timer ─────────────────────────────────────────────────────────────────
    startTimerHz (kUiTimerHz);

    // ── Audio device: 0 inputs, 2 outputs ─────────────────────────────────────
    audioEngine.setAudioChannels (0, 2);

    setSize (700, 240);
}

MainComponent::~MainComponent()
{
    stopTimer();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));

    // Subtle gradient overlay
    juce::ColourGradient gradient (juce::Colour (0xff23233a), 0.0f, 0.0f,
                                   juce::Colour (0xff1a1a2e), 0.0f, (float) getHeight(), false);
    g.setGradientFill (gradient);
    g.fillRect (getLocalBounds());

    // Header
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.setFont (juce::FontOptions (18.0f));
    g.drawText ("DJ Edit Lab - Phase 3: Timing & Quantization",
                getLocalBounds().removeFromTop (44),
                juce::Justification::centred, true);

    // Thin separator line under header
    g.setColour (juce::Colours::white.withAlpha (0.1f));
    g.drawHorizontalLine (44, 20.0f, (float) getWidth() - 20);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (44);   // header
    area.removeFromTop (6);    // gap

    // ── Status + position row ─────────────────────────────────────────────────
    auto infoRow = area.removeFromTop (28);
    positionLabel.setBounds (infoRow.removeFromRight (280));
    statusLabel  .setBounds (infoRow);
    area.removeFromTop (8);

    // ── BPM row ───────────────────────────────────────────────────────────────
    auto bpmRow = area.removeFromTop (30);
    bpmLabel .setBounds (bpmRow.removeFromLeft (40));
    bpmRow.removeFromLeft (6);
    bpmSlider.setBounds (bpmRow);
    area.removeFromTop (10);

    // ── Button row ────────────────────────────────────────────────────────────
    auto buttonRow = area.removeFromTop (46);
    loadFileButton.setBounds (buttonRow.removeFromLeft (130).reduced (0, 4));
    buttonRow.removeFromLeft (10);
    playButton    .setBounds (buttonRow.removeFromLeft (160).reduced (0, 4));
    buttonRow.removeFromLeft (10);
    stopButton    .setBounds (buttonRow.removeFromLeft (80) .reduced (0, 4));
}

//==============================================================================
void MainComponent::timerCallback()
{
    const bool fileLoaded = audioEngine.isFileLoaded();
    const bool playing    = audioEngine.isPlaying();

    playButton.setEnabled (fileLoaded && !playing);
    stopButton.setEnabled (playing);

    if (!fileLoaded)
        return;

    const double sampleRate = audioEngine.getCurrentSampleRate();
    if (sampleRate <= 0.0)
        return;

    auto formatTime = [] (double seconds) -> juce::String
    {
        const int mins = static_cast<int> (seconds) / 60;
        const int secs = static_cast<int> (seconds) % 60;
        return juce::String (mins) + ":" + juce::String (secs).paddedLeft ('0', 2);
    };

    const double  currentSec = static_cast<double> (audioEngine.getPlayheadSamples()) / sampleRate;
    const double  totalSec   = static_cast<double> (audioEngine.getTotalSamples())    / sampleRate;
    const int64_t bar        = audioEngine.getCurrentBar()  + 1;   // 1-indexed for display
    const int     beat       = audioEngine.getCurrentBeat() + 1;

    positionLabel.setText (
        formatTime (currentSec) + " / " + formatTime (totalSec)
        + "  |  Bar " + juce::String (bar) + "  Beat " + juce::String (beat),
        juce::dontSendNotification);
}

//==============================================================================
void MainComponent::loadFileButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Select an audio file to load...",
        juce::File::getSpecialLocation (juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.aiff;*.flac;*.ogg");

    constexpr int chooserFlags = juce::FileBrowserComponent::openMode
                               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& chooser)
    {
        const juce::File chosenFile = chooser.getResult();
        if (!chosenFile.existsAsFile())
            return;

        const bool loaded = audioEngine.loadFile (chosenFile);
        if (loaded)
        {
            loadedFileName = chosenFile.getFileName();
            statusLabel.setText (
                "Loaded: " + loadedFileName
                + "  (BPM: " + juce::String (audioEngine.getBPM(), 1) + ")",
                juce::dontSendNotification);
            playButton.setEnabled (true);
        }
        else
        {
            statusLabel.setText ("Failed to load: " + chosenFile.getFileName(),
                                 juce::dontSendNotification);
        }
    });
}

void MainComponent::playButtonClicked()
{
    // Quantized play — engine schedules start on the next bar boundary
    audioEngine.scheduledPlay();

    // Update the status to show pending state
    statusLabel.setText (
        "Loaded: " + loadedFileName
        + "  (BPM: " + juce::String (audioEngine.getBPM(), 1) + ")  — starting on next bar…",
        juce::dontSendNotification);
}

void MainComponent::stopButtonClicked()
{
    audioEngine.stopPlayback();
}

void MainComponent::bpmSliderChanged()
{
    const double newBPM = bpmSlider.getValue();
    audioEngine.setBPM (newBPM);

    if (audioEngine.isFileLoaded())
    {
        statusLabel.setText (
            "Loaded: " + loadedFileName
            + "  (BPM: " + juce::String (newBPM, 1) + ")",
            juce::dontSendNotification);
    }
}
