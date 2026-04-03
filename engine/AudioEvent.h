#pragma once

#include <cstdint>

/**
    Represents a single scheduled action in the audio engine.

    Events are created on the UI/message thread and pushed into the
    EventScheduler lock-free FIFO.  The audio thread pops them and
    applies them at the exact sample indicated by scheduledSample.

    Realtime safety: this struct contains no heap allocations and
    is trivially copyable — safe to pass through AbstractFifo.
*/
struct AudioEvent
{
    using ID = uint32_t;

    enum class Type : uint8_t
    {
        Play,        ///< Start playback at scheduledSample
        Stop,        ///< Stop playback at scheduledSample
        Seek,        ///< Jump playhead to data.seekTarget
        BPMChange,   ///< Change BPM to data.newBPM (re-syncs grid on next bar)
        SetLoop,     ///< Define and activate a loop region
        ClearLoop,   ///< Deactivate the current loop
    };

    /** Unique monotonic ID — assigned by EventScheduler. */
    ID id { 0 };

    /** Event category. */
    Type type { Type::Play };

    /**
        The absolute sample position at which this event becomes active.
        The audio thread processes the event when its buffer range contains
        this sample.  Use MasterClock::quantizeToBar() to compute the value.
    */
    int64_t scheduledSample { 0 };

    /** True when the event was scheduled against a bar/beat boundary. */
    bool quantized { false };

    /** Type-specific payload — only the relevant member is valid. */
    union Data
    {
        double  newBPM;       ///< valid for BPMChange
        int64_t seekTarget;   ///< valid for Seek (absolute sample)
        
        struct {
            int64_t startSample;
            int64_t lengthSamples;
        } loopArgs;           ///< valid for SetLoop

        Data() : seekTarget (0) {}
    } data;

    //--------------------------------------------------------------------------
    // Priority order when multiple events share the same scheduledSample.
    // Lower value = higher priority.
    static int priority (Type t) noexcept
    {
        switch (t)
        {
            case Type::Stop:      return 0;
            case Type::Seek:      return 1;
            case Type::ClearLoop: return 2;
            case Type::SetLoop:   return 3;
            case Type::BPMChange: return 4;
            case Type::Play:      return 5;
            default:              return 99;
        }
    }
};
