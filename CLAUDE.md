# CLAUDE.md

---

## 🔴 ANTIGRAVITY VERPLICHTE PRE-FLIGHT (LEES DIT EERST)

> Deze sectie is specifiek voor de Antigravity chat-interface.
> Vóór **elke** code-actie, bestandswijziging, of git-commando MOET je:

### 1. Voer `/start-work` uit (workflow in `.agents/workflows/start-work.md`)

Dit is **niet optioneel**. De workflow dwingt:
- ✅ Git branch check (verbod op werken op `main`)
- ✅ Staged/unstaged changes check
- ✅ Vraagclassificatie (triviaal vs. architecturaal)
- ✅ Domeinrouting naar de juiste specialist-regels
- ✅ Vertical slice scope bewaking

### 2. Lees het relevante agent-bestand

Na routing, lees via `view_file` het betreffende `.claude/agents/[domein].md` bestand vóór je implementeert.

### 3. Rapporteer altijd eerst

```
🌿 Branch:   [naam] | [clean / X changes]
📋 Type:     triviaal / architecturaal
🎯 Domein:   [agent(s)]
📦 Scope:    in scope / ⚠️ buiten vertical slice
```

**Ga nooit direct aan de slag zonder dit overzicht te tonen.**

---

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

---

## ⚠️ JUCE String Safety (KRITIEKE REGEL)

**Gebruik NOOIT non-ASCII karakters in string literals die aan `juce::String` worden meegegeven.**

Dit omvat: em dash `—`, smart quotes `"" ''`, checkmark `✔`, en alle andere Unicode > 127.

JUCE's `juce_String.cpp` heeft een `jassert` die crashes veroorzaakt bij non-ASCII bytes.

```cpp
// ❌ VERBODEN — veroorzaakt jassert crash in juce_String.cpp:327
g.drawText ("DJ Edit Lab — Phase 3", ...);
g.drawText ("Status: ✔ OK", ...);

// ✅ CORRECT — alleen ASCII
g.drawText ("DJ Edit Lab - Phase 3", ...);
g.drawText ("Status: OK", ...);
```

**Dit geldt voor:** `drawText()`, `Label::setText()`, `Button::setButtonText()`, en alle andere JUCE string APIs.

---


## Eerste Doel

Zorg dat dit werkt:

- App start
- Audio engine initialised
- Geen crashes
- Stilte output stabiel

Daarna pas:
- audio laden
- playback

---

## Git Workflow Regels (VERPLICHT voor alle agents)

### Trunk-based flow

- `main` is de stabiele trunk — **wijzigingen rechtstreeks op main zijn verboden**
- Werk altijd op een korte, gefocuste branch
- Branches worden zo snel mogelijk gemerged naar main

### Branch Naamgeving

| Type | Prefix | Voorbeeld |
|------|--------|-----------|
| Nieuwe feature | `feature/` | `feature/loop-scheduling` |
| Bugfix | `bugfix/` | `bugfix/crossfade-click` |
| Kritieke fix op main | `hotfix/` | `hotfix/audio-engine-crash` |

### Regels voor Claude

1. **Nooit rechtstreeks committen op `main`** — altijd een branch aanmaken
2. **Twijfel over branch type?** → vraag de gebruiker vóór je doorgaat
3. **Reeds in een feature/bugfix branch en nieuwe feature gevraagd?**
   → Vraag eerst: "Wil je de huidige branch reviewen en mergen naar main, of een nieuwe branch aanmaken?"
4. **Vóór branch switch:** controleer altijd op staged/unstaged changes
   - Zijn er changes? → vraag om te committen, stashen of te resetten
   - Pas daarna switchen
5. **Één concern per branch** — geen twee features mengen

### Verplichte Check vóór Branch Switch

```bash
git status          # Controleer op changes
git stash           # Indien nodig (vraag eerst aan gebruiker)
git checkout -b feature/naam-van-feature
```
