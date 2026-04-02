#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "AudioEvent.h"

#include <array>
#include <atomic>
#include <cstdint>

/**
    Lock-free event scheduler for the DJ Edit Lab audio engine.

    The UI / message thread pushes AudioEvents via scheduleEvent().
    The audio thread pops events via popEventsForBuffer() inside
    getNextAudioBlock — no locks, no allocations.

    Capacity: kFifoSize events in flight simultaneously.

    Thread safety:
      - scheduleEvent()       — call from message thread only
      - popEventsForBuffer()  — call from audio thread only

    The class uses juce::AbstractFifo which provides the correct
    memory ordering guarantees for a single-producer / single-consumer
    scenario (one writing thread, one reading thread).
*/
class EventScheduler
{
public:
    static constexpr int kFifoSize = 64;

    EventScheduler();

    //==========================================================================
    // Message thread

    /**
        Schedule an event.  The event's scheduledSample must already be set
        to the desired absolute sample position (usually computed via
        MasterClock::quantizeToBar).

        Assigns a monotonically increasing ID to the event before storing it.
        Does nothing (silently drops) if the FIFO is full — this should not
        happen in practice given kFifoSize = 64 and typical usage patterns.
    */
    void scheduleEvent (AudioEvent event) noexcept;

    //==========================================================================
    // Audio thread

    /**
        Pops all events whose scheduledSample falls within
        [bufferStart, bufferStart + bufferSize).

        Results are written to the caller-supplied array and sorted by
        scheduledSample (ascending).  Events at the same sample are ordered
        by AudioEvent::priority().

        @param bufferStart  First sample of the current audio buffer
        @param bufferSize   Number of samples in the buffer
        @param result       Caller-owned array to receive events
        @param maxResults   Maximum number of events to return
        @returns            Number of events written to result[]
    */
    int popEventsForBuffer (int64_t        bufferStart,
                            int            bufferSize,
                            AudioEvent*    result,
                            int            maxResults) noexcept;

private:
    // UI -> Audio thread lock-free queue
    juce::AbstractFifo           fifo_;
    std::array<AudioEvent, kFifoSize> storage_;

    std::atomic<AudioEvent::ID> nextId_ { 0 };

    // Audio thread internal future-event buffer
    std::array<AudioEvent, kFifoSize> localQueue_;
    int localQueueSize_ { 0 };
};
