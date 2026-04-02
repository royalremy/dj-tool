#include "TimelineComponent.h"

TimelineComponent::TimelineComponent()
    : thumbnailCache(2),
      thumbnail(512, formatManager, thumbnailCache)
{
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);
}

TimelineComponent::~TimelineComponent()
{
    thumbnail.removeChangeListener(this);
}

void TimelineComponent::setFile(const juce::File& file)
{
    thumbnail.setSource(new juce::FileInputSource(file));
}

void TimelineComponent::setPosition(int64_t playheadSamples, int64_t totalSamples)
{
    if (currentPlayheadSamples != playheadSamples || currentTotalSamples != totalSamples)
    {
        currentPlayheadSamples = playheadSamples;
        currentTotalSamples = totalSamples;
        repaint();
    }
}

void TimelineComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail)
    {
        repaint();
    }
}

void TimelineComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.fillAll(juce::Colour(0xff111122));

    // Thumbnail
    g.setColour(juce::Colour(0xff4a90d9).withAlpha(0.7f)); // Nice blue match with BPM slider
    if (thumbnail.getTotalLength() > 0.0)
    {
        thumbnail.drawChannels(g, bounds, 0.0, thumbnail.getTotalLength(), 1.0f);
    }
    else
    {
        g.setColour(juce::Colours::grey);
        g.setFont(14.0f);
        g.drawText("No audio loaded", bounds, juce::Justification::centred, false);
    }

    // Playhead
    if (currentTotalSamples > 0)
    {
        double proportion = static_cast<double>(currentPlayheadSamples) / currentTotalSamples;
        int playheadX = static_cast<int>(proportion * bounds.getWidth());

        g.setColour(juce::Colours::white);
        g.drawVerticalLine(playheadX, 0.0f, static_cast<float>(bounds.getHeight()));
        
        // Slight glow/thickness for playhead
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawVerticalLine(playheadX - 1, 0.0f, static_cast<float>(bounds.getHeight()));
        g.drawVerticalLine(playheadX + 1, 0.0f, static_cast<float>(bounds.getHeight()));
    }
    
    // Border
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRect(bounds, 1);
}

void TimelineComponent::resized()
{
}
