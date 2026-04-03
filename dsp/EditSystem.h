#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>

namespace dsp
{

constexpr int kFadeSamples = 16; // ~0.3ms at 44100Hz (preserves transients while preventing clicks)

inline float fadeIn (int sample, int totalSamples) noexcept
{
    const float t = static_cast<float> (sample) / static_cast<float> (totalSamples);
    return std::sin (t * juce::MathConstants<float>::halfPi);
}

inline float fadeOut (int sample, int totalSamples) noexcept
{
    const float t = static_cast<float> (sample) / static_cast<float> (totalSamples);
    return std::cos (t * juce::MathConstants<float>::halfPi);
}

struct Edit
{
    int64_t startSample { 0 };
    int64_t endSample { 0 };
    
    enum class Type { Cut, Loop, Gain } type { Type::Cut };
    float parameter { 0.0f }; // loop position, gain level, etc.
};

/**
    Lock-free interval index for editing features.
*/
class EditIndex
{
public:
    static constexpr int kMaxEdits = 500;
    static constexpr int kMaxEditsPerQuery = 32;

    EditIndex() = default;

    void insert (const Edit& edit) noexcept
    {
        if (numEdits < kMaxEdits)
        {
            edits[numEdits++] = edit;
        }
    }

    void clear() noexcept
    {
        numEdits = 0;
    }

    int query (int64_t startRange, int64_t endRange, Edit* results, int maxResults) const noexcept
    {
        int count = 0;
        for (int i = 0; i < numEdits && count < maxResults; ++i)
        {
            const auto& edit = edits[i];
            // Overlap check
            if (edit.startSample < endRange && edit.endSample > startRange)
            {
                results[count++] = edit;
            }
        }
        return count;
    }

    // Overload for querying exactly one point in time
    int query (int64_t sampleTime, Edit* results, int maxResults) const noexcept
    {
        return query (sampleTime, sampleTime + 1, results, maxResults);
    }

private:
    std::array<Edit, kMaxEdits> edits;
    int numEdits { 0 };
};

} // namespace dsp
