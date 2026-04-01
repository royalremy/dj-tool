# DJ Edit Lab — Audio Engine Core Specification v1.3 (Editing, Time-Stretch & Timing)

## 1. Doel

Dit document beschrijft een **production-grade stem editing system** met focus op:
- Click-free audio
- Realtime performance stabiliteit
- Deterministische rendering

---

## 2. Extra Kernprincipes

- Click-free output is verplicht
- Geen discontinuities in waveform
- Realtime-safe (geen allocaties tijdens playback)

---

## 3. Boundary Handling (Anti-Click System)

### 3.1 Probleem

Abrupte amplitude veranderingen veroorzaken clicks:
- Cut boundaries
- Loop restarts
- Gain jumps

### 3.2 Oplossing: Micro Crossfades

Elke edit boundary krijgt automatische fades:

- Fade-in: 5–10 ms
- Fade-out: 5–10 ms

### 3.3 Implementatie

```
fadedSample = sample * fadeCurve(t)
```

Fade curves:
- Default: equal power curve
- Alternatief: linear (fallback)

---

## 4. Loop Stabiliteit

### 4.1 Probleem

Loops veroorzaken:
- clicks op wrap point
- phase discontinuities

### 4.2 Oplossing

- Crossfade tussen loop end en start
- Pre-buffer overlap van 5–10 ms

```
output = (endSegment * fadeOut) + (startSegment * fadeIn)
```

### 4.3 Restricties

- Minimum loop lengte = 1 beat
- Kortere loops worden automatisch gecorrigeerd

---

## 5. Gain Smoothing

### 5.1 Probleem

Plotselinge gain changes → clicks

### 5.2 Oplossing

- Gain transitions worden gesmoothed

```
gain(t) = lerp(previousGain, targetGain, smoothingFactor)
```

- Smoothing window: 5–20 ms

---

## 6. Cut Handling (Improved)

In plaats van harde cut:

- Fade-out aan begin cut
- Fade-in na cut

Gedrag:
- Geen harde nul-overgang
- Stilte blijft behouden

---

## 7. Edit Indexing (Performance Critical)

### 7.1 Probleem

Lineaire scan van edits = te traag

### 7.2 Oplossing: Interval Index

Gebruik:
- Interval Tree of Segment Tree

Functie:
- O(log n) lookup van actieve edits per tijdstip

---

## 8. Realtime Evaluatie (Geoptimaliseerd)

Per buffer chunk:

```
1. Query actieve edits via interval index
2. Cache resultaat voor buffer
3. Apply edits zonder allocatie
```

### 8.1 Caching

- Per buffer caching
- Hergebruik bij stabiele playback

---

## 9. Memory Management

- Geen runtime allocaties in audio thread
- Pre-allocatie van:
  - edit structs
  - buffers

---

## 10. Phase Consistency

### Probleem

- Drums verliezen punch bij edits

### Oplossing

- Preserve phase bij loops
- Geen resampling binnen loop tenzij nodig

---

## 11. Advanced Edge Cases

### 11.1 Overlapping fades

- Combine via multiplicatie

### 11.2 Extreme edit density

- Hard limit: 500 edits/stem

### 11.3 Silence stacking

- Meerdere cuts → één silent block

---

## 12. Determinisme

- Alle fades en smoothing zijn wiskundig bepaald
- Geen random variatie

---

## 13. Resultaat

Dit systeem garandeert:

- Click-free editing
- Stabiele realtime performance
- Voorspelbaar gedrag

---

---

## 14. Time-Stretch Strategie

### 14.1 Doel

Time-stretching maakt tempo-aanpassingen mogelijk zonder pitch te wijzigen, met behoud van realtime performance.

De strategie is bewust gesplitst in:
- Realtime preview (performance first)
- Offline export (quality first)

---

### 14.2 Trigger Momenten

Time-stretch wordt toegepast wanneer:

- BPM wijzigt
- Loop lengte afwijkt van originele timing
- Secties hergebruikt worden met andere timing context

---

### 14.3 Architectuur

Time-stretch wordt **per stem** toegepast, niet op de master mix.

Pipeline:

```
AudioBuffer → Edit System → Time-Stretch Layer → Output
```

Belangrijk:
- Time-stretch gebeurt NA edits
- Niet destructief
- Geen pre-render in preview mode

---

### 14.4 Preview Mode (Realtime)

Doel: stabiele playback met minimale CPU load

Eigenschappen:

- Low-quality time-stretch algoritme
- Lage latency prioriteit
- Mogelijke artifacts toegestaan

Aanbevolen technieken:
- WSOLA (Waveform Similarity Overlap-Add)
- Simplified phase vocoder

Beperkingen:
- Transient smearing mogelijk
- Minder accurate pitch preservation

---

### 14.5 Export Mode (Offline)

Doel: maximale audiokwaliteit

Eigenschappen:

- High-quality algoritme
- Geen realtime beperkingen

Aanbevolen technieken:
- Phase vocoder met transient preservation
- Elastique-achtige aanpak

Extra:
- Transient protection op drums
- Formant preservation op vocals

---

### 14.6 Drums Specifieke Strategie

Omdat drums cruciaal zijn:

- Transients worden gedetecteerd (via drums stem)
- Stretch wordt rond transients geminimaliseerd
- Optioneel: transient locking

Gedrag:
- Transients blijven op grid
- Stretch gebeurt tussen transients

---

### 14.7 Quality Switching

Systeem wisselt automatisch:

| Mode | Stretch Quality |
|------|---------------|
| Preview | Low |
| Export | High |

Geen user control nodig in v1

---

### 14.8 Performance Constraints

Realtime stretch moet voldoen aan:

- Geen buffer underruns
- CPU usage voorspelbaar
- Max 6 stems tegelijk

Fallback gedrag:

- Indien CPU overload:
  - Simplify stretch
  - Of bypass stretch tijdelijk

---

### 14.9 Edge Cases

#### Extreme tempo verandering (> ±20%)

- Preview: artifacts toegestaan
- Export: extra processing

#### Zeer korte loops

- Stretch wordt uitgeschakeld
- Loop wordt direct herhaald

#### Silence regions

- Geen stretch nodig

---

### 14.10 Determinisme

- Stretch algoritmes moeten deterministisch zijn
- Geen random phase variation

---

### 14.11 Samenvatting

Deze strategie zorgt voor:

- Stabiele realtime ervaring
- Hoge exportkwaliteit
- Controle over CPU gebruik

---

---

## 15. Event Scheduling & Timing Spec

### 15.1 Doel

Definieert sample-accurate timing en deterministische event scheduling voor:
- Playback
- Quantized seeks
- Loops
- BPM wijzigingen
- Edit activaties

Focus:
- Geen drift
- Geen timing jitter
- Bar-accurate en sample-accurate gedrag

---

### 15.2 Tijdreferenties

Het systeem gebruikt drie tijdsdomeinen:

1. **Audio Time (seconds)**
   - Continue tijd
   - Gebruikt door audio buffers

2. **Musical Time (bars/beats)**
   - Afgeleid van BPM
   - Gebruikt voor quantization

3. **Sample Time (frames)**
   - Discrete tijd
   - Gebruikt voor rendering

Conversies verlopen via MasterClock.

---

### 15.3 MasterClock

MasterClock is de enige bron van waarheid.

Bevat:
- BPM
- Sample rate
- Start offset

Functies:

```
barsToSeconds()
secondsToBars()
secondsToSamples()
samplesToSeconds()
```

---

### 15.4 Event Model

Alle interacties worden omgezet naar events:

```
Event {
  id: UUID
  type: play | stop | seek | loopStart | loopEnd | bpmChange | editChange
  scheduledTime: sampleTime
  quantized: boolean
}
```

---

### 15.5 Scheduling Strategie

Gebruik **lookahead scheduling**.

Parameters:
- Lookahead window: 50–100 ms
- Scheduling interval: 10–20 ms

Proces:

```
1. Check huidige audioTime
2. Plan events binnen lookahead window
3. Queue events in audio thread
4. Execute exact op sampleTime
```

---

### 15.6 Quantization

Quantization gebeurt in musical time.

Regel:
- Events worden verschoven naar dichtstbijzijnde bar boundary

```
quantizedTime = ceil(currentBar)
```

Conversie:
- bar → seconds → samples

---

### 15.7 Playback Start

- Play wordt altijd quantized gestart
- Start op volgende bar boundary

Edge case:
- Indien al exact op bar → direct starten

---

### 15.8 Seek (Quantized Jump)

Gedrag:
- User klik → doelpositie
- Wordt gequantized naar bar
- Event wordt gepland

Belangrijk:
- Geen onmiddellijke jump
- Altijd via scheduler

---

### 15.9 Loop Scheduling

Loop bestaat uit twee events:
- Loop start
- Loop wrap

Bij wrap:

```
if (currentTime >= loopEnd) {
  jump to loopStart (sample accurate)
}
```

Crossfade wordt toegepast (zie edit system)

---

### 15.10 BPM Changes

Gedrag:
- BPM change wordt gequantized
- Actief vanaf volgende bar

Tijdens overgang:
- Geen abrupt resync
- Smooth transition via MasterClock

---

### 15.11 Drift Preventie

Probleem:
- Floating point accumulatie

Oplossing:
- Gebruik integer sample counters
- Geen accumulatieve timing

```
currentSample += bufferSize
```

---

### 15.12 Audio Thread vs UI Thread

- UI thread triggert events
- Audio thread voert uit

Regel:
- Geen blocking calls in audio thread
- Communicatie via lock-free queue

---

### 15.13 Buffer Processing

Per buffer:

```
1. Bepaal sample range
2. Check events binnen range
3. Split buffer indien nodig
4. Apply events sample-accurate
```

---

### 15.14 Edge Cases

#### Multiple events same timestamp
- Volgorde:
  1. stop
  2. seek
  3. loop
  4. edits

#### Seek tijdens loop
- Loop wordt herberekend

#### BPM change tijdens loop
- Loop boundaries herberekend

---

### 15.15 Determinisme

- Events zijn immutable
- Scheduling is reproduceerbaar
- Geen afhankelijkheid van framerate

---

### 15.16 Resultaat

Dit systeem garandeert:

- Sample-accurate timing
- Strakke loops
- Geen drift
- Consistente quantization

---

**Einde document**

