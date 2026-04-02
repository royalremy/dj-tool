#include "MasterClock.h"

#include <algorithm>

//==============================================================================
void MasterClock::setBPM (double bpm) noexcept
{
    bpm_ = std::clamp (bpm, 20.0, 300.0);
}

void MasterClock::setSampleRate (double sampleRate) noexcept
{
    // Sanity-clamp; a real device will always provide a positive rate.
    sampleRate_ = (sampleRate > 0.0) ? sampleRate : 44100.0;
}

//==============================================================================
double MasterClock::samplesPerBeat() const noexcept
{
    // beats/min → seconds/beat → samples/beat
    // = sampleRate * (60 / BPM)
    return sampleRate_ * (60.0 / bpm_);
}

double MasterClock::samplesPerBar() const noexcept
{
    // 4/4 time: 4 beats per bar
    return samplesPerBeat() * 4.0;
}

//==============================================================================
double MasterClock::barsToSeconds (double bars) const noexcept
{
    return bars * 4.0 * (60.0 / bpm_);
}

double MasterClock::secondsToBars (double seconds) const noexcept
{
    return seconds * (bpm_ / 60.0) / 4.0;
}

int64_t MasterClock::secondsToSamples (double seconds) const noexcept
{
    return static_cast<int64_t> (seconds * sampleRate_);
}

double MasterClock::samplesToSeconds (int64_t samples) const noexcept
{
    return static_cast<double> (samples) / sampleRate_;
}

//==============================================================================
int64_t MasterClock::quantizeToBar (int64_t currentSample) const noexcept
{
    const double spb         = samplesPerBar();
    const double relSamples  = static_cast<double> (currentSample - startOffsetSamples_);
    const double currentBars = relSamples / spb;

    // If we are already (within 1 sample) on a bar boundary, stay here.
    const double fractionalPart = currentBars - std::floor (currentBars);
    if (fractionalPart < (1.0 / spb))
        return currentSample;

    // Otherwise, advance to the start of the next bar.
    const double nextBar       = std::ceil (currentBars);
    const int64_t nextBarSampl = startOffsetSamples_
                                 + static_cast<int64_t> (nextBar * spb);
    return nextBarSampl;
}

int64_t MasterClock::quantizeToBeat (int64_t currentSample) const noexcept
{
    const double spbeat       = samplesPerBeat();
    const double relSamples   = static_cast<double> (currentSample - startOffsetSamples_);
    const double currentBeats = relSamples / spbeat;

    const double fractionalPart = currentBeats - std::floor (currentBeats);
    if (fractionalPart < (1.0 / spbeat))
        return currentSample;

    const double nextBeat        = std::ceil (currentBeats);
    const int64_t nextBeatSampl  = startOffsetSamples_
                                   + static_cast<int64_t> (nextBeat * spbeat);
    return nextBeatSampl;
}

//==============================================================================
int64_t MasterClock::sampleToBar (int64_t sample) const noexcept
{
    const double relSamples = static_cast<double> (sample - startOffsetSamples_);
    return static_cast<int64_t> (relSamples / samplesPerBar());
}

int MasterClock::sampleToBeatInBar (int64_t sample) const noexcept
{
    const double relSamples  = static_cast<double> (sample - startOffsetSamples_);
    const int64_t totalBeats = static_cast<int64_t> (relSamples / samplesPerBeat());
    return static_cast<int> (totalBeats % 4);
}
