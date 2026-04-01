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

---

## Vertical Slice Bouwvolgorde (VERPLICHT)

Dit project wordt gebouwd in strikt opeenvolgende fases. **Een fase mag pas starten als de vorige stabiel is.**

| Fase | Doel | Milestone |
|------|------|-----------|
| 1 | Audio engine fundament | App start, geen crashes, stille output |
| 2 | Audio loading & playback | Track speelt correct af, geen glitches |
| 3 | Timing & quantization | Playback start op bar, timing consistent |
| 4 | Basis UI | UI reageert instant, playhead zichtbaar |
| 5 | Loop functionaliteit | Loop werkt zonder clicks, in sync |
| 6 | Cut functionaliteit | Cut werkt, geen clicks |
| 7 | Multi-stem support (4 stems) | 4 stems spelen synchroon |
| 8 | Stem matrix (basis) | Matrix werkt, mute realtime |
| 9 | UX feedback layer | UI voelt instant, audio blijft quantized |
| 10 | Stabilisatie | Geen glitches, stabiele performance |

**Out of scope voor vertical slice:** AI, 6 stems (→ 4 in slice), advanced beat tools, project management, export.

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
