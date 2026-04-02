#pragma once

#include <cstdint>
#include <cmath>

/**
    MasterClock — single source of truth for all timing in DJ Edit Lab.

    Converts between three time domains:
      - Audio time  : seconds (double, continuous)
      - Sample time : frames  (int64_t, discrete — used in the audio thread)
      - Musical time: bars / beats (derived from BPM)

    All public methods are noexcept and allocation-free — safe to call from
    the realtime audio thread.

    Thread safety:
      - setBPM / setSampleRate are called from the message thread BEFORE
        audio starts, or via a quantized BPMChange event that is applied
        inside the audio callback. They are NOT thread-safe for concurrent
        access; the caller must guarantee exclusive access at that moment.
      - All const query methods (quantizeToBar, samplesPerBar, …) are
        read-only and safe to call from any thread once the clock is
        configured.
*/
class MasterClock
{
public:
    //==========================================================================
    // Configuration — call from message thread or via BPMChange event

    /** Set the BPM. Valid range: [20, 300]. Clamped silently. */
    void setBPM (double bpm) noexcept;

    /** Set the system sample rate (provided by prepareToPlay). */
    void setSampleRate (double sampleRate) noexcept;

    //==========================================================================
    // Grid helpers (noexcept, read-only)

    double getBPM()        const noexcept { return bpm_; }
    double getSampleRate() const noexcept { return sampleRate_; }

    /** Samples per beat (quarter note) at the current BPM and sample rate. */
    double samplesPerBeat() const noexcept;

    /** Samples per bar (4 beats in 4/4). */
    double samplesPerBar()  const noexcept;

    //==========================================================================
    // Conversions

    double  barsToSeconds   (double  bars)    const noexcept;
    double  secondsToBars   (double  seconds) const noexcept;
    int64_t secondsToSamples(double  seconds) const noexcept;
    double  samplesToSeconds(int64_t samples) const noexcept;

    //==========================================================================
    // Quantization

    /**
        Returns the sample position of the NEXT bar boundary strictly after
        currentSample.  If currentSample is already exactly on a bar boundary
        it returns currentSample (i.e., "already aligned — start now").

        @param currentSample  Absolute sample position from start of track
                              (same coordinate as the audio thread counter).
    */
    int64_t quantizeToBar (int64_t currentSample) const noexcept;

    /**
        Returns the sample position of the next beat boundary at or after
        currentSample.
    */
    int64_t quantizeToBeat (int64_t currentSample) const noexcept;

    /**
        Returns the current bar number (0-indexed) for a given sample position.
        Useful for UI display.
    */
    int64_t sampleToBar (int64_t sample) const noexcept;

    /**
        Returns the current beat within the bar (0-indexed, 0–3 in 4/4)
        for a given sample position.
    */
    int sampleToBeatInBar (int64_t sample) const noexcept;

private:
    double  bpm_        { 120.0 };
    double  sampleRate_ { 44100.0 };

    // Sample position that corresponds to bar 0, beat 0.
    // Set when the clock is first started to align the grid to that moment.
    // For Phase 3 this is always 0 (track starts at sample 0).
    int64_t startOffsetSamples_ { 0 };
};
