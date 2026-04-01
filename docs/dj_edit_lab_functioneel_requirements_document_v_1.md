# DJ Edit Lab — Functioneel Requirements Document v1.1

## 1. Overzicht

DJ Edit Lab is een remix-tool voor DJ’s die hun sets voorbereiden. De applicatie is niet bedoeld als live DJ-software, maar als een pre-performance tool waarmee tracks creatief herwerkt worden tot speelklare edits.

AI wordt in versie 1 uitsluitend gebruikt voor analyse (stems, BPM, structuur). Automatische remix-generatie is buiten scope.

---

## 2. Workflow Model

De applicatie werkt volgens een state-based workflow met vrije navigatie.

**States:**
- Imported
- Analysed
- Edited
- Exported

De gebruiker kan op elk moment terugkeren naar een vorige state zonder dat data verloren gaat.

---

## 3. Globale Regels

- Alle bewerkingen zijn non-destructief en ongedaan maakbaar tenzij anders vermeld
- Laatste bewerking krijgt altijd prioriteit bij conflicten

---

## 4. Grid Systeem

- Alle bewerkingen zijn gebaseerd op een globale bar- en beat-grid
- Grid-resolutie is instelbaar (1/1 t.e.m. 1/64)
- Alle interacties snappen naar dit grid
- Grid schaalt dynamisch met zoomniveau

---

## 5. Analyse

### 5.1 Analyse bij import

- BPM detectie (decimaal)
- Toonaard detectie
- Sectie detectie
- Stem-scheiding (6 stems):
  - Drums
  - Bass
  - Vocals
  - Melody/Synth
  - FX
  - Other

### 5.2 Transient Detectie

- Detectie gebeurt uitsluitend op de **Drums-stem**
- Vormt basis voor beatgrid en quantization
- Threshold instelbaar

### 5.3 Error Handling

- Stem-scheiding faalt → fallback naar originele track
- BPM onzeker → user bevestiging vereist
- Corrupt bestand → import geweigerd
- Analyse opnieuw uitvoerbaar

---

## 6. Audio Engine

### 6.1 Model

De applicatie gebruikt een **event-based audio engine**.

Audio wordt aangestuurd via geplande events:
- Play
- Stop
- Quantized seek
- Loop start/stop
- Stem mute/solo

### 6.2 Architectuur

```
Project
 ├── Track
 │     ├── Stems[6]
 │     │     ├── AudioBuffer
 │     │     ├── EditLayers[]
 │     │
 │     ├── Sections[]
 │     │     ├── StartTime
 │     │     ├── EndTime
 │     │     ├── Metadata
 │
 ├── MasterClock
 │     ├── BPM
 │     ├── TimeGrid
 │
 ├── PlaybackEngine (realtime)
 ├── RenderEngine (export)
```

- Sections zijn referenties, geen audio copies
- Edits zijn lagen bovenop originele audio

### 6.3 Kwaliteitsmodi

**Preview mode (realtime):**
- Lage latency prioriteit
- Lagere audio kwaliteit toegestaan

**Export mode:**
- Maximale kwaliteit
- Offline rendering

---

## 7. Latency Requirements

| Actie            | Max latency |
|------------------|------------|
| Mute toggle      | <50ms      |
| Solo             | <50ms      |
| Play start       | <100ms     |
| Seek (quantized) | 1 bar max  |
| Loop activation  | <100ms     |

---

## 8. Playback Gedrag

- Playhead volgt master clock
- Klik in timeline → **quantized jump naar dichtstbijzijnde bar**
- Geen free seek tijdens playback

### Edge cases

- Klik tijdens loop → nieuwe positie wordt volgende loop boundary
- Seek buiten track → genegeerd
- BPM change tijdens playback → her-synchronisatie op volgende bar

---

## 9. Editing

### 9.1 Cut

- Selectie wordt verwijderd
- Wordt vervangen door stilte
- Geen verschuiving van audio
- Tracklengte blijft gelijk

### 9.2 Loop

- Loop snapt aan grid
- Onjuiste lengte → auto-correct of user confirm

### Conflict Rules

- Loop overschrijft bestaande audio in regio
- Cut na loop → cut wint
- Nieuwe edit overschrijft oude edit

---

## 10. Structuur (Secties)

- Secties zijn analyse-eenheden (intro, break, drop, etc.)
- Werken als tijdsreferenties

### Gedrag

- Drag & drop herschikt alleen metadata (geen audio copy)
- Splits/merge beïnvloedt alleen sectie-definitie

---

## 11. Stem Controle

### Globaal

- Mute
- Solo
- Volume

### Per sectie

- Matrix systeem

### Conflict Rules

- Timeline edit > matrix
- Laatste actie wint

---

## 12. Performance Constraints

- Werkt op macOS met 8GB RAM
- Max track length: 10 minuten
- Max 6 stems actief

---

## 13. Undo/Redo

- Standaard: 20 stappen
- Instelbaar tot max 20

---

## 14. Export

- WAV / MP3
- Offline rendering

---

## 15. Scope v1

- Geen AI remix generatie
- Geen live DJ functionaliteit
- Geen cloud

---

**Einde document**

