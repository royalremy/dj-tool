#include "AudioEngine.h"

//==============================================================================
AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
    shutdownAudio();
}

//==============================================================================
void AudioEngine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // Initialize DSP and buffers here in later phases
}

void AudioEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // 1. Clear buffer (always the first step in realtime callback)
    bufferToFill.clearActiveBufferRegion();

    if (!playing.load())
        return;

    // 2. Logic for playback will be added in Phase 2
}

void AudioEngine::releaseResources()
{
}

//==============================================================================
void AudioEngine::startPlayback()
{
    playing.store (true);
}

void AudioEngine::stopPlayback()
{
    playing.store (false);
}
