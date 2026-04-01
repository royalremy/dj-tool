#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // ── Status label ──────────────────────────────────────────────────────────
    statusLabel.setText ("No file loaded.", juce::dontSendNotification);
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    statusLabel.setFont (juce::FontOptions (14.0f));
    addAndMakeVisible (statusLabel);

    // ── Position label ────────────────────────────────────────────────────────
    positionLabel.setText ("0:00 / 0:00", juce::dontSendNotification);
    positionLabel.setJustificationType (juce::Justification::centredRight);
    positionLabel.setFont (juce::FontOptions (14.0f));
    addAndMakeVisible (positionLabel);

    // ── Load File button ──────────────────────────────────────────────────────
    loadFileButton.onClick = [this] { loadFileButtonClicked(); };
    addAndMakeVisible (loadFileButton);

    // ── Play / Stop button ────────────────────────────────────────────────────
    playStopButton.setEnabled (false);   // disabled until a file is loaded
    playStopButton.onClick = [this] { playStopButtonClicked(); };
    addAndMakeVisible (playStopButton);

    // ── Start the UI refresh timer ─────────────────────────────────────────────
    startTimerHz (kUiTimerHz);

    // ── Audio device: 0 inputs, 2 outputs ─────────────────────────────────────
    audioEngine.setAudioChannels (0, 2);

    setSize (600, 200);
}

MainComponent::~MainComponent()
{
    stopTimer();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));   // dark navy background

    // Header text
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.setFont (juce::FontOptions (18.0f));
    g.drawText ("DJ Edit Lab — Phase 2: Audio Loading & Playback",
                getLocalBounds().removeFromTop (40),
                juce::Justification::centred,
                true);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (20);

    // Reserve header space
    area.removeFromTop (40);

    // Status + position row
    auto infoRow = area.removeFromTop (30);
    positionLabel .setBounds (infoRow.removeFromRight (120));
    statusLabel   .setBounds (infoRow);

    area.removeFromTop (10);

    // Button row
    auto buttonRow = area.removeFromTop (50);
    loadFileButton.setBounds (buttonRow.removeFromLeft (140).reduced (0, 5));
    buttonRow.removeFromLeft (10);
    playStopButton.setBounds (buttonRow.removeFromLeft (100).reduced (0, 5));
}

//==============================================================================
void MainComponent::timerCallback()
{
    // This runs on the message thread — safe to read atomics from audio engine

    updatePlayStopButton();

    if (!audioEngine.isFileLoaded())
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

    const double currentSec = static_cast<double> (audioEngine.getPlayheadSamples()) / sampleRate;
    const double totalSec   = static_cast<double> (audioEngine.getTotalSamples())    / sampleRate;

    positionLabel.setText (formatTime (currentSec) + " / " + formatTime (totalSec),
                           juce::dontSendNotification);
}

//==============================================================================
void MainComponent::loadFileButtonClicked()
{
    // FileChooser must stay alive until the async callback completes
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
            statusLabel.setText ("Loaded: " + loadedFileName,
                                 juce::dontSendNotification);
            playStopButton.setEnabled (true);
        }
        else
        {
            statusLabel.setText ("Failed to load: " + chosenFile.getFileName(),
                                 juce::dontSendNotification);
        }
    });
}

void MainComponent::playStopButtonClicked()
{
    if (audioEngine.isPlaying())
        audioEngine.stopPlayback();
    else
        audioEngine.startPlayback();

    updatePlayStopButton();
}

void MainComponent::updatePlayStopButton()
{
    playStopButton.setButtonText (audioEngine.isPlaying() ? "Stop" : "Play");
}
