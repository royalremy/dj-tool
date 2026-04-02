#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class TimelineComponent : public juce::Component,
                          public juce::ChangeListener
{
public:
    TimelineComponent();
    ~TimelineComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void setFile(const juce::File& file);
    void setPosition(int64_t playheadSamples, int64_t totalSamples);

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    int64_t currentPlayheadSamples { 0 };
    int64_t currentTotalSamples { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};
