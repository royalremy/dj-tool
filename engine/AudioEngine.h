#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>
#include <memory>

/**
    Core Audio Engine for DJ Edit Lab — Phase 2: Audio Loading & Playback.

    Realtime safety rules:
      - loadFile() MUST be called on the message thread only (disk I/O)
      - startPlayback() / stopPlayback() delegate to AudioTransportSource (lock-free)
      - getNextAudioBlock delegates entirely to transportSource (no allocations)
*/
class AudioEngine : public juce::AudioAppComponent
{
public:
    AudioEngine();
    ~AudioEngine() override;

    //==============================================================================
    // AudioAppComponent overrides
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    // Transport control — safe to call from message thread
    void startPlayback();
    void stopPlayback();

    /**
        Loads a WAV or MP3 file from disk.
        MUST be called on the message thread — performs disk I/O.
        Returns true if the file was loaded successfully.
    */
    bool loadFile (const juce::File& file);

    //==============================================================================
    // State queries (lock-free reads)
    bool isPlaying() const noexcept { return transportSource.isPlaying(); }
    bool isFileLoaded() const noexcept { return fileLoaded.load(); }

    /** Current playhead position in samples. Written by audio thread, read by UI. */
    int64_t getPlayheadSamples() const noexcept { return playheadSamples.load(); }

    /** Total length of loaded file in samples. Set on message thread after load. */
    int64_t getTotalSamples() const noexcept { return totalSamples.load(); }

    /** Sample rate set during prepareToPlay. */
    double getCurrentSampleRate() const noexcept { return currentSampleRate.load(); }

private:
    //==============================================================================
    // File loading (message thread only)
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    //==============================================================================
    // Lock-free state shared between audio thread and UI thread
    std::atomic<bool>    fileLoaded      { false };
    std::atomic<int64_t> playheadSamples { 0 };
    std::atomic<int64_t> totalSamples    { 0 };
    std::atomic<double>  currentSampleRate { 44100.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
};
