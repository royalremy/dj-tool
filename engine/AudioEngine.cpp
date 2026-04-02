#include "AudioEngine.h"

//==============================================================================
AudioEngine::AudioEngine()
{
    formatManager.registerBasicFormats();
}

AudioEngine::~AudioEngine()
{
    transportSource.stop();
    transportSource.setSource (nullptr);
    shutdownAudio();
}

//==============================================================================
void AudioEngine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate.store (sampleRate);

    // Initialise MasterClock with the confirmed system sample rate.
    masterClock_.setSampleRate (sampleRate);
    masterClock_.setBPM (bpm_.load());

    transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void AudioEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // 1. Clear buffer — always first
    bufferToFill.clearActiveBufferRegion();

    if (!fileLoaded.load())
        return;

    // 2. Determine the sample range covered by this buffer.
    //    We track the playhead as an absolute sample counter driven by the
    //    AudioTransportSource position (avoids our own drift).
    const double sampleRate     = currentSampleRate.load();
    const int64_t bufferStart   = static_cast<int64_t> (
        transportSource.getCurrentPosition() * sampleRate);
    const int      bufferSize   = bufferToFill.numSamples;

    // 3. Pop any events scheduled within [bufferStart, bufferStart + bufferSize).
    static constexpr int kMaxEventsPerBuffer = 16;
    AudioEvent events[kMaxEventsPerBuffer];
    const int eventCount = scheduler_.popEventsForBuffer (
        bufferStart, bufferSize, events, kMaxEventsPerBuffer);

    // 4. Fill audio.
    //    If there are no intra-buffer events, just fill the whole block.
    //    If there are events, split the buffer at each event boundary.
    if (eventCount == 0)
    {
        transportSource.getNextAudioBlock (bufferToFill);
    }
    else
    {
        int processedSamples = 0;

        for (int e = 0; e < eventCount; ++e)
        {
            const AudioEvent& ev      = events[e];
            const int splitPoint      = static_cast<int> (ev.scheduledSample - bufferStart);
            const int samplesToRender = splitPoint - processedSamples;

            // Render the segment before this event
            if (samplesToRender > 0)
            {
                juce::AudioSourceChannelInfo segment (bufferToFill);
                segment.startSample += processedSamples;
                segment.numSamples   = samplesToRender;
                transportSource.getNextAudioBlock (segment);
                processedSamples += samplesToRender;
            }

            // Apply the event
            applyEvent (ev);
        }

        // Render remaining samples after last event
        const int remaining = bufferSize - processedSamples;
        if (remaining > 0)
        {
            juce::AudioSourceChannelInfo segment (bufferToFill);
            segment.startSample += processedSamples;
            segment.numSamples   = remaining;
            transportSource.getNextAudioBlock (segment);
        }
    }

    // 5. Update playhead and musical position atomics (read by UI thread)
    const int64_t newPlayhead = static_cast<int64_t> (
        transportSource.getCurrentPosition() * sampleRate);
    playheadSamples.store (newPlayhead);
    currentBar .store (masterClock_.sampleToBar (newPlayhead));
    currentBeat.store (masterClock_.sampleToBeatInBar (newPlayhead));
}

void AudioEngine::releaseResources()
{
    transportSource.releaseResources();
}

//==============================================================================
bool AudioEngine::loadFile (const juce::File& file)
{
    jassert (juce::MessageManager::existsAndIsCurrentThread());

    auto* reader = formatManager.createReaderFor (file);
    if (reader == nullptr)
        return false;

    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);

    transportSource.setSource (
        newSource.get(),
        0,
        nullptr,
        reader->sampleRate,
        2);

    totalSamples.store (reader->lengthInSamples);
    readerSource = std::move (newSource);
    fileLoaded.store (true);
    transportSource.setPosition (0.0);

    return true;
}

//==============================================================================
void AudioEngine::scheduledPlay()
{
    // Compute the current playhead in samples to find the next bar boundary.
    const double  sampleRate   = currentSampleRate.load();
    const int64_t currentSmp   = static_cast<int64_t> (
        transportSource.getCurrentPosition() * sampleRate);

    const int64_t targetSample = masterClock_.quantizeToBar (currentSmp);

    AudioEvent ev;
    ev.type            = AudioEvent::Type::Play;
    ev.scheduledSample = targetSample;
    ev.quantized       = true;

    scheduler_.scheduleEvent (ev);
}

void AudioEngine::stopPlayback()
{
    // Stop is immediate — no quantization required.
    transportSource.stop();
}

//==============================================================================
void AudioEngine::setBPM (double bpm) noexcept
{
    AudioEvent ev;
    ev.type = AudioEvent::Type::BPMChange;
    ev.data.newBPM = bpm;
    // Dispatched directly to start next buffer window
    ev.scheduledSample = playheadSamples.load() + 1; // apply almost immediately
    scheduler_.scheduleEvent (ev);
}

//==============================================================================
// Private — audio thread only
void AudioEngine::applyEvent (const AudioEvent& event) noexcept
{
    switch (event.type)
    {
        case AudioEvent::Type::Play:
            transportSource.start();
            break;

        case AudioEvent::Type::Stop:
            transportSource.stop();
            break;

        case AudioEvent::Type::Seek:
        {
            const double targetSeconds = static_cast<double> (event.data.seekTarget)
                                         / currentSampleRate.load();
            // NOTE: setPosition is not strictly realtime-safe in all JUCE versions;
            // for Phase 3 this is acceptable. Phase 5+ should use a dedicated seek
            // ring buffer with pre-loaded read-ahead.
            transportSource.setPosition (targetSeconds);
            break;
        }

        case AudioEvent::Type::BPMChange:
            masterClock_.setBPM (event.data.newBPM);
            bpm_.store (event.data.newBPM);
            break;

        default:
            break;
    }
}
