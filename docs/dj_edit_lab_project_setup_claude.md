# DJ Edit Lab — Functioneel & UX Requirements Document v1.2 (Complete)

## 1. Overzicht

DJ Edit Lab is een remix-tool voor DJ’s die hun sets voorbereiden. De applicatie is bedoeld om tracks creatief te herwerken tot speelklare edits.

Focus:
- Realtime preview
- Creatieve controle per stem en per sectie
- Snelle, intuïtieve workflow

---

## 2. Workflow Model

States:
- Imported
- Analysed
- Edited
- Exported

Vrije navigatie tussen states.

---

## 3. Globale Regels

- Alle bewerkingen zijn non-destructief
- Laatste bewerking wint
- Realtime performance heeft prioriteit

---

## 4. Grid Systeem

- Bar/beat grid als basis
- Resolutie: 1/1 → 1/64
- Alles snapt naar grid

---

## 5. Analyse

- BPM detectie
- Toonaard detectie
- Sectie detectie
- 6 stems

### Transients

- Alleen drums

### Correctie UX

- Tap tempo
- Secties hernoemen
- Secties splitsen/mergen
- Vrije labels mogelijk

---

## 6. Structuur (Secties)

Secties zijn tijdsreferenties (intro, drop, etc.)

### Functionaliteit

- Drag & drop reorder
- Dupliceren
- Knippen
- Samenvoegen
- Splitsen
- Nieuwe sectie maken

---

## 7. Stem x Sectie Matrix

Core interface:

Grid:
- Rijen = stems
- Kolommen = secties

Per cel:
- Mute toggle
- Volume controle

### Interacties

- Multi-select (shift/cmd)
- Visuele status (actief/inactief)

---

## 8. Timeline Editing

### Selectie

- Click & drag
- Min lengte: 1 beat
- Mag over meerdere secties lopen

### Acties

- Cut
- Loop
- Gain

Action buttons verschijnen bij selectie

---

## 9. Editing Gedrag

### Cut

- Verwijdert audio
- Vervangt door stilte
- Geen verschuiving

### Loop

- Snapt aan grid
- Auto-correct indien fout

---

## 10. Beat Tools

- Auto quantize (instelbaar)
- Beat locking
- Multi-beat selectie
- Swing
- Humanize

---

## 11. Playback UX

- Quantized playback
- Loop preview toggle
- Snap-to-bar gedrag zichtbaar

---

## 12. Project Management

### Structuur

/project
- original/
- stems/
- project.json

### Functionaliteit

- Autosave (5 min)
- Recent projects
- Rename
- Duplicate
- Delete

---

## 13. Export

- WAV / MP3
- Bitrate selectie
- Sample rate selectie
- Progress indicator
- Show in Finder

---

## 14. Keyboard Shortcuts

- Play/Pause
- Undo/Redo
- Loop toggle
- Mute/Solo
- Zoom

---

## 15. Performance Constraints

- 8GB RAM
- 10 min tracks
- Max 6 stems

---

## 16. Error Handling

- Stem fail → fallback
- BPM onzeker → user confirm
- Corrupt file → reject

---

## 17. Non-Functional

- Import: WAV, MP3, FLAC
- Export: WAV, MP3
- Autosave verplicht
- Crash safety
- Keyboard-first usability

---

## 18. Belangrijke Opmerking over andere documenten

Ja — de andere documenten moeten gedeeltelijk geüpdatet worden:

### Moet aangepast worden:

1. **UX Spec**
- Matrix interacties toevoegen
- Section editing feedback

2. **Editing System Spec**
- Moet expliciet rekening houden met:
  - section-based overrides
  - matrix vs timeline conflicts

3. **Timing Spec**
- Moet rekening houden met:
  - section reorder events
  - loop preview UX

### Moet NIET aangepast worden:

- Time-stretch spec (blijft correct)
- Audio core architectuur (blijft correct)

---

## 19. Samenvatting

Dit document herstelt:

- Volledige UX
- Alle user-facing features
- Project lifecycle

En behoudt:

- Technische correctheid
- Performance focus

---

**Einde document**

