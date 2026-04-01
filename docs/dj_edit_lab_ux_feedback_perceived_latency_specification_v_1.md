# DJ Edit Lab — UX Feedback & Perceived Latency Specification v1.0

## 1. Doel

Definieert hoe de applicatie **direct responsief aanvoelt** ondanks quantized audio scheduling.

Focus:
- Perceived latency minimaliseren
- Visuele en tactiele feedback synchroniseren met audio
- Gebruikersvertrouwen verhogen

---

## 2. Kernprincipe

> Audio mag vertraagd zijn (quantized), maar feedback nooit.

Er is een expliciete scheiding:

| Component | Gedrag |
|----------|--------|
| Audio | Quantized, gepland |
| UI feedback | Instant (0–16ms) |

---

## 3. Interaction Feedback Model

Elke user actie heeft 3 fases:

1. **Input (0 ms)**
2. **Immediate UI Feedback (<16 ms)**
3. **Scheduled Audio Response (quantized)**

---

## 4. Actie-specifiek gedrag

### 4.1 Play

- Knop verandert onmiddellijk naar “active”
- Playhead preview animatie start direct
- Audio start op volgende bar

### 4.2 Seek (Quantized Jump)

- Ghost playhead springt onmiddellijk naar target
- Echte playhead volgt bij audio start
- Target positie visueel gemarkeerd

### 4.3 Loop Activation

- Loop region highlight direct zichtbaar
- Countdown (optioneel) tot activatie
- Audio loop start op bar boundary

### 4.4 Mute / Solo

- Knopstatus verandert direct
- Metering reflecteert target state
- Audio mute volgt binnen latency budget

---

## 5. Visual Feedback System

### 5.1 Dual Playhead

- **Real playhead** → audio positie
- **Preview playhead** → user intent

Gedrag:
- Preview reageert instant
- Real volgt scheduler

---

### 5.2 Pending State Indicators

Voor geplande acties:

- Pulsing highlight
- Countdown bar (optioneel)
- Subtiele animatie

---

### 5.3 Grid Feedback

- Snap wordt visueel getoond vóór commit
- Hover toont exacte drop locatie

---

## 6. Perceived Latency Targets

| Component | Target |
|----------|--------|
| UI response | <16 ms |
| Visual feedback | <16 ms |
| Audio reaction | conform timing spec |

---

## 7. Conflict & Edge Case UX

### 7.1 Rapid Input (spam clicks)

- Alleen laatste input wordt behouden
- Vorige pending events worden overschreven

### 7.2 Multiple Actions Pending

- UI toont alleen laatste state
- Geen stacking van visuele states

### 7.3 Seek tijdens loop

- Loop highlight update direct
- Nieuwe loop actief vanaf volgende boundary

### 7.4 BPM Change

- UI update direct
- Visual grid reflow instant
- Audio volgt op volgende bar

---

## 8. Error Feedback

- Ongeldige acties → directe visuele feedback
- Geen blocking dialogs tijdens playback

Voorbeelden:
- Loop te kort → highlight + auto-correct
- Edit buiten bereik → genegeerd + subtle indicator

---

## 9. Motion & Animatie

- Animaties max 150 ms
- Geen easing die timing maskeert
- Functioneel boven esthetisch

---

## 10. Audio-Visual Sync

- Visuals volgen audio, niet omgekeerd
- Geen fake sync

---

## 11. Resultaat

Dit systeem zorgt voor:

- Directe responsiviteit
- Begrijpbaar gedrag
- Vertrouwen in timing

---

**Einde document**

