#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "MasterClock.h"
#include "EventScheduler.h"

#include <atomic>
#include <memory>

/**
    Core Audio Engine for DJ Edit Lab — Phase 3: Timing & Quantization.

    Realtime safety rules:
      - loadFile()      MUST be called on the message thread only (disk I/O)
      - scheduledPlay() pushes a Play event into the lock-free EventScheduler
      - stopPlayback()  stops the transport directly (immediate, no quantization)
      - getNextAudioBlock processes scheduled events at exact sample boundaries

    New in Phase 3:
      - MasterClock provides BPM / bar / sample conversions
      - EventScheduler delivers events to the audio thread without locks
      - Play events are quantized to the next bar boundary by default
*/
class AudioEngine : public juce::AudioAppComponent
{
public:
    AudioEngine();
    ~AudioEngine() override;

    //==========================================================================
    // AudioAppComponent overrides
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==========================================================================
    // Transport control — safe to call from the message thread

    /**
        Schedule a quantized play start.
        The engine computes the next bar boundary from the current playhead
        position and schedules a Play event there via the EventScheduler.
        If the playhead is already on a bar boundary, playback starts immediately.
    */
    void scheduledPlay();

    /**
        Stop playback immediately (no quantization).
        Safe to call from the message thread — delegates to AudioTransportSource.
    */
    void stopPlayback();

    /**
        Loads a WAV or MP3 file from disk.
        MUST be called on the message thread — performs disk I/O.
        Returns true if the file was loaded successfully.
        Resets the playhead and bar counter to zero.
    */
    bool loadFile (const juce::File& file);

    //==========================================================================
    // Clock / timing — message thread setters

    /** Set the BPM [20–300]. Updates MasterClock immediately. */
    void setBPM (double bpm) noexcept;

    /** Current BPM (lock-free read). */
    double getBPM() const noexcept { return bpm_.load(); }

    //==========================================================================
    // State queries (lock-free reads, safe from any thread)

    bool    isPlaying()        const noexcept { return transportSource.isPlaying(); }
    bool    isFileLoaded()     const noexcept { return fileLoaded.load(); }

    /** Current playhead in samples (written by audio thread). */
    int64_t getPlayheadSamples() const noexcept { return playheadSamples.load(); }

    /** Total file length in samples. */
    int64_t getTotalSamples()    const noexcept { return totalSamples.load(); }

    /** Sample rate set during prepareToPlay. */
    double  getCurrentSampleRate() const noexcept { return currentSampleRate.load(); }

    /** Current bar number (0-indexed), updated by audio thread. */
    int64_t getCurrentBar()  const noexcept { return currentBar.load(); }

    /** Current beat within the bar (0-indexed, 0–3 in 4/4). */
    int     getCurrentBeat() const noexcept { return currentBeat.load(); }

private:
    //==========================================================================
    // Audio-thread helpers (noexcept, no allocations, called from getNextAudioBlock)
    void applyEvent (const AudioEvent& event) noexcept;

    //==========================================================================
    // File loading (message thread only)
    juce::AudioFormatManager                   formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource                 transportSource;

    //==========================================================================
    // Timing subsystem
    MasterClock    masterClock_;
    EventScheduler scheduler_;

    //==========================================================================
    // Lock-free state shared between audio thread and UI thread
    std::atomic<bool>    fileLoaded       { false };
    std::atomic<int64_t> playheadSamples  { 0 };
    std::atomic<int64_t> totalSamples     { 0 };
    std::atomic<double>  currentSampleRate { 44100.0 };
    std::atomic<double>  bpm_             { 120.0 };
    std::atomic<int64_t> currentBar       { 0 };
    std::atomic<int>     currentBeat      { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
};
