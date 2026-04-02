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
    // Peek at everything in the FIFO without committing a read yet.
    // We only consume events that belong to the current buffer window.
    const int available = fifo_.getNumReady();
    if (available == 0 || maxResults == 0)
        return 0;

    const int64_t bufferEnd = bufferStart + static_cast<int64_t> (bufferSize);
    int collected = 0;

    // We need to iterate the ring buffer to find matching events.
    // Use AbstractFifo::read() in a peek-style loop:
    //   - Read one at a time; put back events that don't belong to this buffer.
    // Because AbstractFifo doesn't support peek-and-requeue natively, we drain
    // all available events into a small temporary stack buffer, return those that
    // match, and re-schedule the rest.
    //
    // Stack limit: kFifoSize entries — no heap allocation.
    AudioEvent tempBuffer[kFifoSize];
    int tempCount = 0;

    {
        const auto readScope = fifo_.read (available);

        for (int i = 0; i < readScope.blockSize1 && tempCount < kFifoSize; ++i)
            tempBuffer[tempCount++] = storage_[static_cast<size_t> (readScope.startIndex1 + i)];

        for (int i = 0; i < readScope.blockSize2 && tempCount < kFifoSize; ++i)
            tempBuffer[tempCount++] = storage_[static_cast<size_t> (readScope.startIndex2 + i)];
    }

    // Partition: events for this buffer vs events for a later buffer
    for (int i = 0; i < tempCount; ++i)
    {
        const AudioEvent& ev = tempBuffer[i];

        if (ev.scheduledSample >= bufferStart && ev.scheduledSample < bufferEnd)
        {
            // Belongs to this buffer window
            if (collected < maxResults)
                result[collected++] = ev;
            // (extra events beyond maxResults are silently discarded — shouldn't happen)
        }
        else
        {
            // Belongs to a future buffer — put it back
            scheduleEvent (ev);
        }
    }

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
