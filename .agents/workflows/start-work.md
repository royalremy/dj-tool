---
description: Orchestrator pre-flight check — altijd uitvoeren vóór code werk in dj-tool
---

# DJ Edit Lab — Orchestrator Pre-Flight Workflow

// turbo-all

Voer deze stappen altijd uit vóór je code schrijft, bestanden aanmaakt, of git-acties uitvoert in het `dj-tool` project.

## Stap 1: Git Pre-Flight Check

// turbo
1. Voer uit:
```bash
git -C /Users/remy/Documents/Code/dj-tool branch --show-current
git -C /Users/remy/Documents/Code/dj-tool status --short
```

Interpreteer de output:
- **Op `main`?** → Stop. Informeer de gebruiker. Maak een branch aan met de juiste prefix (`feature/`, `bugfix/`, of `hotfix/`) vóór je verdergaat.
- **Op een feature/bugfix branch?** → Ga door naar stap 2.
- **Staged of unstaged changes aanwezig?** → Vraag de gebruiker: "Wil je de openstaande changes committen, stashen, of resetten vóór ik verderdga?"

## Stap 2: Vraagclassificatie (Orchestrator-logica)

Analyseer de vraag van de gebruiker en classificeer expliciet:

### ✅ Triviaal
- Geïsoleerde bugfix binnen één subsysteem
- Parameter aanpassing (threshold, fade, grid)
- UI styling of layout tweak
- Bestaand gedrag uitbreiden zonder nieuwe lagen

→ Ga direct naar stap 3 (domeinrouting)

### 🏛️ Architecturaal
- Nieuwe feature die >1 subsysteem raakt
- Nieuw object, module of thread introduceren
- API-grens hertekenen
- Realtime safety risico

→ Meld dit expliciet en vraag goedkeuring van de gebruiker vóór je implementeert. Beschrijf het architectuurplan en wacht op akkoord.

## Stap 3: Domeinrouting

Bepaal het domein op basis van de vraag en pas de regels uit het corresponderende agent-bestand toe:

| Domein | Triggers | Agent-regels toe te passen |
|--------|----------|---------------------------|
| **Git** | branches, commits, merges, stash | `.claude/agents/git-workflow.md` |
| **Audio Engine** | `getNextAudioBlock`, `prepareToPlay`, audio thread, buffer, playback | `.claude/agents/audio-engine.md` |
| **Edit Systeem** | cut, loop, edit, fade, crossfade, boundary, interval | `.claude/agents/edit-system.md` |
| **Timing & Clock** | BPM, bar, beat, quantize, MasterClock, scheduling, drift | `.claude/agents/timing-clock.md` |
| **Stem Pipeline** | stem, drums, bass, vocals, mute, solo, AudioBuffer | `.claude/agents/stem-pipeline.md` |
| **UI/UX** | UI, component, paint, resized, playhead, animatie, knop | `.claude/agents/ui-component.md` |
| **Build & Infra** | CMake, build, compile, linker, JUCE, dependency | `.claude/agents/build-infra.md` |
| **Architect** | Alles wat architecturaal is (zie stap 2) | `.claude/agents/architect.md` |

Lees het relevante agent-bestand in `.claude/agents/` via de `view_file` tool vóór je implementeert.

## Stap 4: Vertical Slice Bewaking

Controleer of de gevraagde feature binnen de **huidige scope** valt:

> **Huidige scope (vertical slice):** 1 track, max 4 stems, basis UI, play/stop, loop, cut.

❌ Features buiten scope (AI, 6 stems, export, project management) worden **niet** opgepakt zonder expliciete goedkeuring van de gebruiker.

## Stap 5: Rapporteer vóór je implementeert

Toon altijd dit kort overzicht vóór de eerste code:

```
🌿 Branch:   [naam] | [clean / X changes]
📋 Type:     triviaal / architecturaal
🎯 Domein:   [agent(s)]
📦 Scope:    in scope / ⚠️ buiten vertical slice
```

Pas daarna ga je implementeren.
