# DJ Edit Lab — Vertical Slice Development Plan v1.0

## 1. Doel

Dit plan definieert een **strakke build volgorde** om zo snel mogelijk een werkende vertical slice te realiseren.

Vertical slice = minimale maar volledige flow:
- track laden
- playback
- basis editing
- UI feedback

---

## 2. Scope van de Vertical Slice

### In scope

- 1 track
- Max 4 stems (vereenvoudigd)
- Basis waveform visualisatie
- Play / pause (quantized)
- Simple loop
- Simple cut
- Basis UI

### Out of scope (later)

- AI
- 6 stems
- Advanced beat tools
- Project management
- Export

---

## 3. Fase 1 — Audio Engine Fundament

### Doel

Stabiele realtime audio callback zonder glitches

### Taken

1. JUCE project opstarten
2. Audio device initialiseren
3. getNextAudioBlock stabiel laten draaien
4. Silence output

### Milestone 1

✔ App start
✔ Geen crashes
✔ Audio thread stabiel

---

## 4. Fase 2 — Audio Loading & Playback

### Doel

Track laden en afspelen

### Taken

1. WAV/MP3 file loader
2. AudioBuffer vullen
3. Playback cursor (sample counter)
4. Basic play/stop

### Milestone 2

✔ Track speelt correct af
✔ Geen glitches

---

## 5. Fase 3 — Timing & Quantization

### Doel

Strakke playback timing

### Taken

1. MasterClock implementatie
2. BPM detectie (mock/manual)
3. Bar grid berekening
4. Quantized play start

### Milestone 3

✔ Playback start op bar
✔ Timing consistent

---

## 6. Fase 4 — Basis UI

### Doel

Minimale controle-interface

### Taken

1. Play button
2. Basic waveform view
3. Playhead visualisatie

### Milestone 4

✔ UI reageert instant
✔ Playhead zichtbaar

---

## 7. Fase 5 — Loop Functionaliteit

### Doel

Eerste editing feature

### Taken

1. Loop selectie (fixed length)
2. Loop scheduling
3. Crossfade implementatie

### Milestone 5

✔ Loop werkt zonder clicks
✔ Blijft in sync

---

## 8. Fase 6 — Cut Functionaliteit

### Doel

Tweede editing feature

### Taken

1. Selectie maken
2. Cut toepassen (silence)
3. Fade in/out boundaries

### Milestone 6

✔ Cut werkt
✔ Geen clicks

---

## 9. Fase 7 — Multi-Stem Support (4 stems)

### Doel

Basis stem-systeem

### Taken

1. Meerdere buffers laden
2. Sync tussen stems
3. Mixdown in output

### Milestone 7

✔ 4 stems spelen synchroon

---

## 10. Fase 8 — Stem Matrix (Basis)

### Doel

Core UX introduceren

### Taken

1. Grid UI (stems x sections)
2. Mute per stem
3. Basis visuele feedback

### Milestone 8

✔ Matrix werkt
✔ Mute realtime

---

## 11. Fase 9 — UX Feedback Layer

### Doel

Perceived latency oplossen

### Taken

1. Dual playhead (preview + real)
2. Immediate button feedback
3. Pending state indicators

### Milestone 9

✔ UI voelt instant
✔ Audio blijft quantized

---

## 12. Fase 10 — Stabilisatie

### Doel

Engine betrouwbaar maken

### Taken

1. Edge cases testen
2. Performance check (CPU/RAM)
3. Bug fixes

### Milestone 10

✔ Geen glitches
✔ Stabiele performance

---

## 13. Bouwvolgorde Samenvatting

1. Audio engine (must)
2. Playback (must)
3. Timing (must)
4. UI (basic)
5. Loop
6. Cut
7. Multi-stem
8. Matrix
9. UX polish
10. Stabilisatie

---

## 14. Kritische Regels

- Nooit meerdere features tegelijk bouwen
- Elke fase moet stabiel zijn voor volgende start
- Audio thread altijd prioriteit

---

## 15. Resultaat

Na deze slice heb je:

- Werkende audio engine
- Basis editing
- Functionele UI

Dit is de fundering voor alle verdere features.

---

**Einde document**

