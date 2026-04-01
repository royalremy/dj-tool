#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>

/**
    Core Audio Engine for DJ Edit Lab.
    Follows real-time safety constraints.
*/
class AudioEngine : public juce::AudioAppComponent
{
public:
    AudioEngine();
    ~AudioEngine() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void startPlayback();
    void stopPlayback();
    bool isPlaying() const { return playing.load(); }

private:
    std::atomic<bool> playing { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
};
