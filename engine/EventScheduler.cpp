#include "EventScheduler.h"

#include <algorithm>

//==============================================================================
EventScheduler::EventScheduler()
    : fifo_ (kFifoSize)
{
    // Pre-zero the storage so valgrind / ASan are happy
    storage_.fill (AudioEvent{});
}

//==============================================================================
void EventScheduler::scheduleEvent (AudioEvent event) noexcept
{
    event.id = nextId_.fetch_add (1, std::memory_order_relaxed);

    // AbstractFifo::write() is a two-phase operation; we use the RAII scope helper.
    const int numToWrite = 1;
    const auto writeScope = fifo_.write (numToWrite);

    if (writeScope.blockSize1 > 0)
        storage_[static_cast<size_t> (writeScope.startIndex1)] = event;
    else if (writeScope.blockSize2 > 0)
        storage_[static_cast<size_t> (writeScope.startIndex2)] = event;
    // If blockSize1 == 0 and blockSize2 == 0, the FIFO is full — event is dropped.
    // This is guarded by the kFifoSize = 64 capacity; in practice it won't happen.
}

//==============================================================================
int EventScheduler::popEventsForBuffer (int64_t     bufferStart,
                                        int         bufferSize,
                                        AudioEvent* result,
                                        int         maxResults) noexcept
{
    // We drain the entire lock-free FIFO and merge it into our local thread queue
    {
        const int available = fifo_.getNumReady();
        if (available > 0)
        {
            const auto readScope = fifo_.read (available);

            for (int i = 0; i < readScope.blockSize1 && localQueueSize_ < kFifoSize; ++i)
                localQueue_[localQueueSize_++] = storage_[static_cast<size_t> (readScope.startIndex1 + i)];

            for (int i = 0; i < readScope.blockSize2 && localQueueSize_ < kFifoSize; ++i)
                localQueue_[localQueueSize_++] = storage_[static_cast<size_t> (readScope.startIndex2 + i)];
        }
    }

    if (localQueueSize_ == 0 || maxResults == 0)
        return 0;

    const int64_t bufferEnd = bufferStart + static_cast<int64_t> (bufferSize);
    int collected = 0;

    // Partition: events for this buffer vs events for a later buffer
    // Keep events that belong in the future around in the localQueue_
    int writeIdx = 0;
    for (int i = 0; i < localQueueSize_; ++i)
    {
        const AudioEvent& ev = localQueue_[i];

        if (ev.scheduledSample >= bufferStart && ev.scheduledSample < bufferEnd)
        {
            // Belongs to this buffer window
            if (collected < maxResults)
                result[collected++] = ev;
            // (extra events beyond maxResults are silently discarded)
        }
        else
        {
            // Future event (or stray past event) — keep in local queue
            localQueue_[writeIdx++] = ev;
        }
    }
    localQueueSize_ = writeIdx;

    // Sort results by (scheduledSample, priority) so the audio thread can split
    // buffers in order without extra bookkeeping.
    std::sort (result, result + collected, [] (const AudioEvent& a, const AudioEvent& b)
    {
        if (a.scheduledSample != b.scheduledSample)
            return a.scheduledSample < b.scheduledSample;

        return AudioEvent::priority (a.type) < AudioEvent::priority (b.type);
    });

    return collected;
}
