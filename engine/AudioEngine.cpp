#include "AudioEngine.h"

//==============================================================================
AudioEngine::AudioEngine()
{
    // Register all standard audio formats (WAV, AIFF, MP3, OGG, FLAC, …)
    formatManager.registerBasicFormats();
}

AudioEngine::~AudioEngine()
{
    // Must stop transport before shutdown to avoid use-after-free on audio thread
    transportSource.stop();
    transportSource.setSource (nullptr);
    shutdownAudio();
}

//==============================================================================
void AudioEngine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // Cache sample rate for UI queries (lock-free)
    currentSampleRate.store (sampleRate);

    // AudioTransportSource handles its own internal buffering and resampling.
    // No allocation happens here beyond what JUCE pre-allocates internally.
    transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void AudioEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // 1. Clear buffer — always the first step in the realtime callback
    bufferToFill.clearActiveBufferRegion();

    if (!fileLoaded.load())
        return;

    // 2. Fill buffer via AudioTransportSource (internally lock-free, no allocations)
    transportSource.getNextAudioBlock (bufferToFill);

    // 3. Update playhead position (atomic write — read by UI for visualisation)
    //    getCurrentPosition() is a simple atomic read inside JUCE, safe here.
    playheadSamples.store (
        static_cast<int64_t> (transportSource.getCurrentPosition()
                              * currentSampleRate.load()));
}

void AudioEngine::releaseResources()
{
    transportSource.releaseResources();
}

//==============================================================================
bool AudioEngine::loadFile (const juce::File& file)
{
    // MUST run on the message thread — createReaderFor performs disk I/O.
    jassert (juce::MessageManager::existsAndIsCurrentThread());

    // Create a format reader for the file
    auto* reader = formatManager.createReaderFor (file);
    if (reader == nullptr)
        return false;

    // Hand reader to a new AudioFormatReaderSource (takes ownership)
    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);

    // Tell transport source about the new input.
    // The second arg (true) tells it to take ownership.
    transportSource.setSource (
        newSource.get(),
        0,                      // readAheadBufferSize (0 = sync read, simpler for Phase 2)
        nullptr,                // backgroundThread (none needed for sync read)
        reader->sampleRate,     // source sample rate
        2);                     // max output channels

    // Cache total length for UI progress bar / waveform
    totalSamples.store (reader->lengthInSamples);

    // Keep the source alive (transport source holds a raw ptr, we hold the owner)
    readerSource = std::move (newSource);

    // Signal audio thread that a file is ready (lock-free)
    fileLoaded.store (true);

    // Reset playhead to start
    transportSource.setPosition (0.0);

    return true;
}

//==============================================================================
void AudioEngine::startPlayback()
{
    transportSource.start();
}

void AudioEngine::stopPlayback()
{
    transportSource.stop();
}
