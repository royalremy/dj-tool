#include "AudioEngine.h"

//==============================================================================
AudioEngine::AudioEngine()
{
    formatManager.registerBasicFormats();
}

AudioEngine::~AudioEngine()
{
    shutdownAudio();
}

//==============================================================================
void AudioEngine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate.store (sampleRate);

    // Initialise MasterClock with the confirmed system sample rate.
    masterClock_.setSampleRate (sampleRate);
    masterClock_.setBPM (bpm_.load());

    crossfadeTailBuffer_.setSize (2, dsp::kFadeSamples);
    crossfadeTailBuffer_.clear();
    crossfadeRemaining_ = 0;

    engineSampleTime.store (0);
}

void AudioEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // 1. Clear buffer — always first
    bufferToFill.clearActiveBufferRegion();

    if (!fileLoaded.load())
        return;

    // 2. Determine the sample range covered by this buffer on the Master timeline.
    const double sampleRate     = currentSampleRate.load();
    const int64_t bufferStart   = engineSampleTime.load();
    const int      bufferSize   = bufferToFill.numSamples;

    // 3. Pop any events scheduled within [bufferStart, bufferStart + bufferSize).
    static constexpr int kMaxEventsPerBuffer = 16;
    AudioEvent events[kMaxEventsPerBuffer];
    const int eventCount = scheduler_.popEventsForBuffer (
        bufferStart, bufferSize, events, kMaxEventsPerBuffer);

    // 4. Fill audio.
    //    If there are no intra-buffer events, just fill the whole block.
    //    If there are events, split the buffer at each event boundary.
    if (eventCount == 0)
    {
        renderNextBlockWithLoops (bufferToFill);
    }
    else
    {
        int processedSamples = 0;

        for (int e = 0; e < eventCount; ++e)
        {
            const AudioEvent& ev      = events[e];
            const int splitPoint      = static_cast<int> (ev.scheduledSample - bufferStart);
            const int samplesToRender = splitPoint - processedSamples;

            // Render the segment before this event
            if (samplesToRender > 0)
            {
                juce::AudioSourceChannelInfo segment (bufferToFill);
                segment.startSample += processedSamples;
                segment.numSamples   = samplesToRender;
                renderNextBlockWithLoops (segment);
                processedSamples += samplesToRender;
            }

            // Apply the event
            applyEvent (ev);
        }

        // Render remaining samples after last event
        const int remaining = bufferSize - processedSamples;
        if (remaining > 0)
        {
            juce::AudioSourceChannelInfo segment (bufferToFill);
            segment.startSample += processedSamples;
            segment.numSamples   = remaining;
            renderNextBlockWithLoops (segment);
        }
    }

    // 5. Update engine master timeline
    engineSampleTime.fetch_add (bufferSize);

    // 6. Update musical position atomics (read by UI thread, using exactly counted playheadSamples)
    const int64_t currentPlayhead = playheadSamples.load();
    currentBar .store (masterClock_.sampleToBar (currentPlayhead));
    currentBeat.store (masterClock_.sampleToBeatInBar (currentPlayhead));
}

void AudioEngine::releaseResources()
{
}

//==============================================================================
bool AudioEngine::loadFile (const juce::File& file)
{
    jassert (juce::MessageManager::existsAndIsCurrentThread());

    auto* reader = formatManager.createReaderFor (file);
    if (reader == nullptr)
        return false;

    memoryBuffer.setSize (reader->numChannels, static_cast<int> (reader->lengthInSamples));
    reader->read (&memoryBuffer, 0, static_cast<int> (reader->lengthInSamples), 0, true, true);

    totalSamples.store (reader->lengthInSamples);
    fileLoaded.store (true);
    playheadSamples.store (0);
    isPlaying_.store (false);

    delete reader;
    return true;
}

//==============================================================================
void AudioEngine::scheduledPlay()
{
    // Compute next bar boundary based on the absolute engine timeline!
    const int64_t currentEngineSmp = engineSampleTime.load();
    const int64_t targetSample     = masterClock_.quantizeToBar (currentEngineSmp);

    AudioEvent ev;
    ev.type            = AudioEvent::Type::Play;
    ev.scheduledSample = targetSample;
    ev.quantized       = true;

    scheduler_.scheduleEvent (ev);
}

void AudioEngine::stopPlayback()
{
    // Stop is immediate on the timeline — no quantization required.
    AudioEvent ev;
    ev.type = AudioEvent::Type::Stop;
    ev.scheduledSample = engineSampleTime.load() + 1;
    scheduler_.scheduleEvent (ev);
}

//==============================================================================
void AudioEngine::setBPM (double bpm) noexcept
{
    AudioEvent ev;
    ev.type = AudioEvent::Type::BPMChange;
    ev.data.newBPM = bpm;
    // Dispatch effectively immediately on the timeline
    ev.scheduledSample = engineSampleTime.load() + 1;
    scheduler_.scheduleEvent (ev);
}

void AudioEngine::setLoop(int numBars)
{ 
    const int64_t currentPlayhead = getPlayheadSamples();
    int64_t startSample = masterClock_.quantizeToBar(currentPlayhead);
    
    // If the playhead is already well past the previous bar (e.g., jump backwards would happen),
    // and we hit loop, we should probably loop from the next bar to preserve continuous playback.
    // In actual use, setting a loop generally activates it from the *last* bar. We'll use the
    // bar preceding currentPlayhead + some margin.
    int64_t curBarStart = masterClock_.secondsToSamples(masterClock_.barsToSeconds(masterClock_.sampleToBar(currentPlayhead)));
    startSample = curBarStart;
    
    int64_t lengthSamples = static_cast<int64_t>(numBars * masterClock_.samplesPerBar());

    AudioEvent ev;
    ev.type = AudioEvent::Type::SetLoop;
    ev.scheduledSample = engineSampleTime.load() + 1; // apply immediately
    ev.quantized = false;
    ev.data.loopArgs.startSample = startSample;
    ev.data.loopArgs.lengthSamples = lengthSamples;
    
    scheduler_.scheduleEvent (ev);
}

void AudioEngine::clearLoop()
{
    AudioEvent ev;
    ev.type = AudioEvent::Type::ClearLoop;
    ev.scheduledSample = engineSampleTime.load() + 1;
    scheduler_.scheduleEvent (ev);
}

//==============================================================================
// Private — audio thread only
void AudioEngine::applyEvent (const AudioEvent& event) noexcept
{
    switch (event.type)
    {
        case AudioEvent::Type::Play:
            isPlaying_.store (true);
            break;

        case AudioEvent::Type::Stop:
            isPlaying_.store (false);
            break;

        case AudioEvent::Type::Seek:
        {
            playheadSamples.store (event.data.seekTarget);
            break;
        }

        case AudioEvent::Type::BPMChange:
            masterClock_.setBPM (event.data.newBPM);
            bpm_.store (event.data.newBPM);
            break;

        case AudioEvent::Type::SetLoop:
            loopStartSamples.store (event.data.loopArgs.startSample);
            loopLengthSamples.store (event.data.loopArgs.lengthSamples);
            loopActive.store (true);
            
            editIndex_.clear();
            {
                dsp::Edit loopEdit;
                loopEdit.type = dsp::Edit::Type::Loop;
                loopEdit.startSample = event.data.loopArgs.startSample;
                loopEdit.endSample   = event.data.loopArgs.startSample + event.data.loopArgs.lengthSamples;
                loopEdit.parameter   = 0.0f;
                editIndex_.insert (loopEdit);
            }
            break;

        case AudioEvent::Type::ClearLoop:
            loopActive.store (false);
            editIndex_.clear();
            break;

        default:
            break;
    }
}

void AudioEngine::readFromMemory (juce::AudioSourceChannelInfo& segment, int64_t startSampleInFile) noexcept
{
    if (memoryBuffer.getNumSamples() == 0 || !isPlaying_.load())
    {
        segment.clearActiveBufferRegion();
        return;
    }

    const int available = memoryBuffer.getNumSamples() - static_cast<int>(startSampleInFile);
    int toCopy = juce::jmin (segment.numSamples, available);
    
    if (toCopy > 0)
    {
        for (int ch = 0; ch < segment.buffer->getNumChannels(); ++ch)
        {
            const int readCh = juce::jmin (ch, memoryBuffer.getNumChannels() - 1);
            segment.buffer->copyFrom (ch, segment.startSample,
                                      memoryBuffer, readCh,
                                      static_cast<int>(startSampleInFile), toCopy);
        }
    }
    
    if (toCopy < segment.numSamples)
    {
        for (int ch = 0; ch < segment.buffer->getNumChannels(); ++ch)
            segment.buffer->clear (ch, segment.startSample + toCopy, segment.numSamples - toCopy);

        isPlaying_.store(false);
    }
}

void AudioEngine::renderNextBlockWithLoops (const juce::AudioSourceChannelInfo& originalSegment) noexcept
{
    if (!isPlaying_.load() || memoryBuffer.getNumSamples() == 0)
    {
        originalSegment.clearActiveBufferRegion();
        return;
    }

    int processedSamples = 0;
    int64_t localPlayhead = playheadSamples.load();

    while (processedSamples < originalSegment.numSamples)
    {
        juce::AudioSourceChannelInfo segment (originalSegment);
        segment.startSample += processedSamples;
        segment.numSamples -= processedSamples;
        
        if (crossfadeRemaining_ > 0)
        {
            int toRender = juce::jmin (segment.numSamples, crossfadeRemaining_);
            segment.numSamples = toRender;
            
            readFromMemory (segment, localPlayhead);
            
            // Apply crossfade
            const int fadeOffset = dsp::kFadeSamples - crossfadeRemaining_;
            for (int ch = 0; ch < segment.buffer->getNumChannels(); ++ch)
            {
                float* outData = segment.buffer->getWritePointer (ch, segment.startSample);
                const int tailCh = juce::jmin (ch, crossfadeTailBuffer_.getNumChannels() - 1);
                const float* tailData = crossfadeTailBuffer_.getReadPointer (tailCh, fadeOffset);
                
                for (int i = 0; i < toRender; ++i)
                {
                    float incoming = outData[i];
                    float tailSamp = tailData[i];
                    int t = fadeOffset + i;
                    outData[i] = tailSamp * dsp::fadeOut (t, dsp::kFadeSamples) 
                               + incoming * dsp::fadeIn (t, dsp::kFadeSamples);
                }
            }
            
            crossfadeRemaining_ -= toRender;
            processedSamples += toRender;
            localPlayhead += toRender;
            continue;
        }

        int toRender = segment.numSamples;
        bool triggerLoop = false;
        
        if (loopActive.load())
        {
            int64_t turnaroundPoint = loopStartSamples.load() + loopLengthSamples.load() - dsp::kFadeSamples;
            if (localPlayhead <= turnaroundPoint && localPlayhead + toRender > turnaroundPoint)
            {
                toRender = static_cast<int> (turnaroundPoint - localPlayhead);
                if (toRender == 0)
                {
                    triggerLoop = true;
                }
            }
        }
        
        if (triggerLoop)
        {
            juce::AudioSourceChannelInfo tailSegment;
            tailSegment.buffer = &crossfadeTailBuffer_;
            tailSegment.startSample = 0;
            tailSegment.numSamples = dsp::kFadeSamples;
            
            readFromMemory (tailSegment, localPlayhead);
            
            localPlayhead = loopStartSamples.load();
            crossfadeRemaining_ = dsp::kFadeSamples;
            continue;
        }
        else
        {
            segment.numSamples = toRender;
            readFromMemory (segment, localPlayhead);
            processedSamples += toRender;
            localPlayhead += toRender;
        }
    }
    
    playheadSamples.store (localPlayhead);
}

