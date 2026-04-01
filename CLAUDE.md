# CLAUDE.md

## Project Context

Dit project is een macOS audio applicatie gebouwd met:
- C++
- JUCE framework
- Realtime audio engine

Focus:
- Lage latency
- Deterministisch gedrag
- Geen glitches of clicks

---

## Belangrijke Regels

1. NOOIT:
   - geheugen allocaties in audio thread
   - locks of mutexes in audio callbacks
   - blocking calls

2. ALTIJD:
   - realtime-safe code schrijven
   - stack allocation gebruiken waar mogelijk
   - duidelijke scheiding tussen UI en audio

---

## Audio Engine Regels

- getNextAudioBlock is realtime
- geen logging in audio thread
- geen STL containers aanpassen tijdens playback

---

## Edit System Regels

- edits zijn non-destructief
- laatste edit wint
- gebruik indexing (geen lineaire scans)

---

## Timing Regels

- events zijn sample-accurate
- gebruik integer sample counters
- geen floating drift

---

## Verwachtingen van Claude

- Schrijf production-grade code
- Vermijd simplistische oplossingen
- Respecteer realtime constraints
- Als iets niet realtime-safe is: expliciet vermelden

---

## Code Style

- Modern C++ (C++17)
- Duidelijke naming
- Geen magic numbers

```

---

## 8. Eerste Doel

Zorg dat dit werkt:

- App start
- Audio engine initialised
- Geen crashes
- Stilte output stabiel

Daarna pas:
- audio laden
- playback
