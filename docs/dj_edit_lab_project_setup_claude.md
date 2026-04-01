# DJ Edit Lab — Project Setup & CLAUDE.md

## 1. Doel

Concrete setup om onmiddellijk te starten met ontwikkeling:
- JUCE + C++ audio engine
- CMake build
- VS Code + Claude Code workflow

---

## 2. Folder Structuur

```
DJEditLab/
 ├── CMakeLists.txt
 ├── /app
 │    ├── main.cpp
 │
 ├── /engine
 │    ├── AudioEngine.h
 │    ├── AudioEngine.cpp
 │
 ├── /dsp
 │    ├── EditSystem.h
 │    ├── EditSystem.cpp
 │
 ├── /ui
 │    ├── MainComponent.h
 │    ├── MainComponent.cpp
 │
 ├── /third_party
 │
 ├── /build
```

---

## 3. CMake Setup (basis)

```
cmake_minimum_required(VERSION 3.15)
project(DJEditLab)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(JUCE)

juce_add_gui_app(DJEditLab
    PRODUCT_NAME "DJ Edit Lab"
)

juce_generate_juce_header(DJEditLab)

target_sources(DJEditLab PRIVATE
    app/main.cpp
    engine/AudioEngine.cpp
    dsp/EditSystem.cpp
    ui/MainComponent.cpp
)

target_link_libraries(DJEditLab PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)
```

---

## 4. Basic Audio Engine Skeleton

### AudioEngine.h

```
#pragma once
#include <JuceHeader.h>

class AudioEngine : public juce::AudioAppComponent
{
public:
    AudioEngine();
    ~AudioEngine() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

private:
    double currentSampleRate = 44100.0;
};
```

### AudioEngine.cpp

```
#include "AudioEngine.h"

AudioEngine::AudioEngine()
{
    setAudioChannels (0, 2);
}

AudioEngine::~AudioEngine()
{
    shutdownAudio();
}

void AudioEngine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
}

void AudioEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
}

void AudioEngine::releaseResources()
{
}
```

---

## 5. Main UI Component

### MainComponent.h

```
#pragma once
#include <JuceHeader.h>
#include "../engine/AudioEngine.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AudioEngine audioEngine;
};
```

---

## 6. VS Code Setup

Installeer:
- CMake Tools
- C/C++ Extension

Configureer:
- Compiler: Apple Clang
- Build folder: /build

---

**Einde document**

