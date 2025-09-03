# PlaygroundGames — CPX/CPB No-Screen Launcher + Mini Games (Pack 3)

> Adafruit Circuit Playground **Express** (SAMD21) & **Bluefruit** (nRF52840)  
> One boot-time selector + three arcade-style games (each with an alt-mode), tuned for the 10-LED NeoPixel ring, buttons, and accelerometer.

- **Launcher (boot picker)** — choose once at power-up:
  - **A held** → **Reaction Timer** → **Tilt Maze** (alt-mode via **B** while in Reaction)
  - **B held** → **Rapid Fire** → **Shaker Challenge** (alt-mode via **B** while in Rapid)
  - **No buttons** → **Tug-of-War** (default; Solo/2-Player chosen from idle)
- **In-game universal:** **Hold A+B ≈1.5s** to reset the *current game* back to its idle (does **not** return to the launcher).

## Table of Contents
- [Features](#features)
- [Hardware](#hardware)
- [Install & Build](#install--build)
- [Quick Start](#quick-start)
- [Launcher UX](#launcher-ux)
- [Games](#games)
  - [Tug-of-War (default)](#tug-of-war-default)
  - [Reaction Timer](#reaction-timer)
    - [Alt-Mode: Tilt Maze](#alt-mode-tilt-maze)
  - [Rapid Fire](#rapid-fire)
    - [Alt-Mode: Shaker Challenge](#alt-mode-shaker-challenge)
- [Tuning & Constants](#tuning--constants)
- [Code Structure](#code-structure)
- [Troubleshooting](#troubleshooting)
- [FAQ](#faq)
- [License](#license)
- [Credits](#credits)

---

## Features
- **No-screen** arcade games using only the CPX/CPB **NeoPixel ring**, **A/B buttons**, and **accelerometer**.
- **Single boot selection**: pick a game in a ~1.5s window, then run it until hardware reset.
- **Polished feedback**: breathing idles, confirm spinners, tones, win/lose flashes.
- **Alt-modes** accessed in-game (press **B**), so a single binary offers multiple play styles.
- **Robust inputs**: debounced A+B long-press reset and “release guard” between phases.

---

## Hardware
- **Boards:**  
  - Adafruit **Circuit Playground Express** (SAMD21)  
  - Adafruit **Circuit Playground Bluefruit** (nRF52840)
- **Sensors used:**  
  - 10x NeoPixel ring  
  - Left/Right buttons (A/B)  
  - 3-axis accelerometer
- **Libraries:**  
  - **Adafruit Circuit Playground** (install via Arduino Library Manager)

No external wiring required.

---

## Install & Build
1. In **Arduino IDE**, install board defs for CPX/CPB.
2. **Library Manager** → install **Adafruit Circuit Playground**.
3. Open `cp_minigames_3.ino`.
4. Select board + port, then **Upload**.
5. On reboot you’ll see a brief **spinner/hints** window to choose a game.

---

## Quick Start
- **At boot (~1.5s):**
  - Hold **A** → **Reaction Timer** (green spinner)
  - Hold **B** → **Rapid Fire** (blue spinner)
  - Hold **none** → **Tug-of-War** (amber default)
- **Any game:** hold **A+B ~1.5s** → reset to that game’s idle (two quick blinks confirm).

---

## Launcher UX
Runs **once at boot**:
- Holding **A** shows a **green** spinner (Reaction).
- Holding **B** shows a **blue** spinner (Rapid).
- Holding **none** shows **amber** hints (Tug default).
- After the window, the picked game stays active until a hardware reset.

---

## Games

### Tug-of-War (default)
**Flow:** *Idle breathe* → *Solo or 2-Player match* → *Back to idle*

- **Idle:** Amber breathing.  
  - **Press A** → **Solo vs CPU** (you pull left with **A**).  
  - **Press B** → **Two-Player** (Left uses **A**, Right uses the **Right** button).
- **Gameplay:** Each button press pulls the marker one step around the ring. Reach your side’s end to win.  
- **Feedback:** Ticks of tone per pull; win flashes your color (green = left, blue = right).

**Controls**
- **A** (idle) → start **Solo**  
- **B** (idle) → start **Two-Player**  
- **A** (in match) → pull left  
- **Right button** (in match) → pull right (CPU pulses probabilistically in Solo)  
- **A+B (hold)** → reset Tug idle

---

### Reaction Timer
**Flow:** *Idle breathe* → *Arm (tap A)* → *Random wait* → *GO* → *Score display* → *Idle*

- **Idle:** Teal breathing.
- **Arm:** **Tap A** to arm. The ring turns **blue** (“wait…”).  
  - After a random delay, the board flashes **green** + tone (**GO**).
- **React:** Press **A or Right** as fast as possible.
- **Scoring:**  
  - **Hundreds of ms** → amber flashes (count the blinks)  
  - **Ones of 10ms** → green pips (0–9 LEDs lit)

**False Start**
- Pressing a button **before** GO → red flash + penalty, then back to idle.

**Controls**
- **A** (idle) → arm  
- **Any button** on GO → stop timer  
- **B** (idle) → enter **Tilt Maze** (alt-mode)  
- **A+B (hold)** → reset to Reaction idle

#### Alt-Mode: Tilt Maze
- **Goal:** Tilt to steer a blue “ball” to successive **amber checkpoints** around the ring (fixed path).  
- **Rule:** Stay near each checkpoint; if you drift away too long you fail (red) and restart.  
- **Feedback:** Short rising tone on each checkpoint; brief green flash on success.

**Controls**
- **Tilt** to steer  
- **A+B (hold)** → exit back to Reaction idle

---

### Rapid Fire
**Flow:** *Idle breathe* → *Timed heat* → *Win/Lose* → *Idle*

- **Idle:** Purple breathing. **Press A** to start a heat.
- **Heat:** Mash **A** to fill the green bar to **10 LEDs** before time runs out.  
  - The bar **decays** automatically at a fixed cadence.

**Defaults (from code)**
- `RF_ROUND_MS = 10000` (10s heat)  
- `RF_DECAY_MS = 450` (bar decays ≈2 LEDs every ~0.9s on average)  
- `RF_GOAL = 10` (fill the ring)

**Controls**
- **A** (idle) → start heat  
- **A** (in heat) → increase bar  
- **B** (idle) → **Shaker Challenge** (alt-mode)  
- **A+B (hold)** → reset Rapid idle

#### Alt-Mode: Shaker Challenge
**Flow:** *Ready pulse* → *Shake to build energy vs decay* → *Hold near-full to win* → *Idle*

- **What you see:** A **teal meter** from 0–10 LEDs reflecting motion energy.  
- **How to win:** Get to **≥9 LEDs** and keep it there for **1.5s**.  
- **Time limit:** **12s** per round.  
- **Audio:** Near full, you’ll hear soft encouragement chirps; win = triple green flash + tones.

**What “counts” as a shake (from code):**
- The game computes **g-excess** = `| |accel| − 1g |` each tick.  
- Only motion above `SHK_THRESH = 1.2` (≈0.12 g) adds energy; it decays by `SHK_EN_DECAY = 0.985` each tick.  
- Energy gain per tick above threshold is `ge * SHK_EN_FROM_G = ge * 0.045`.  
- Meter mapping: about `energy*6 + 0.5` LEDs, clamped to 0–10.

**Controls**
- **Shake** to build/maintain the meter  
- **A+B (hold)** → exit back to Rapid idle

---

## Tuning & Constants

Edit these at the top of `cp_minigames_3.ino`:

### Global
- `BRIGHTNESS` — NeoPixel brightness (0–255)  
- `LAUNCHER_WINDOW_MS` — boot selection window (ms)  
- `AB_RESET_MS` — A+B hold time to reset in-game (ms)

### Reaction Timer
- `REACT_MIN_DELAY_MS` / `REACT_MAX_DELAY_MS` — random wait before GO  
- `REACT_FALSE_PENALTY_MS` — penalty on false start

### Tilt Maze (alt for Reaction)
- `TILT_STEP_MS` — update cadence (ms)  
- `TILT_LOSE_MS` — max time away from checkpoint (ms)  
- `TILT_GAIN` — tilt sensitivity

### Rapid Fire
- `RF_ROUND_MS` — heat length (ms)  
- `RF_DECAY_MS` — decay cadence (↑ value = **slower** decay)  
- `RF_GOAL` — LEDs required to win

### Shaker Challenge (alt for Rapid)
- `SHK_THRESH` — minimum g-excess to count as shake (lower = easier)  
- `SHK_EN_FROM_G` — energy added per unit g-excess  
- `SHK_EN_DECAY` — per-tick energy drain (closer to 1.0 = slower drain)  
- `SHK_TICK_MS` — update cadence (ms)  
- `SHK_ROUND_MS` — total round time (ms)  
- `SHK_HOLD_MS` — time to hold ≥9 LEDs (ms)

### Tug-of-War
- `TUG_TRACK_LEN` — distance from center to win (LEDs)  
- `TUG_STEP_MS` — drawing cadence (ms)  
- **Solo CPU:**  
  - `CPU_PULSE_MS` — CPU attempt period (ms)  
  - `CPU_PUSH_CHANCE` — chance (%) to push on each pulse  
  - `SOLO_ROUND_MS` — solo timeout (ms)

---

## Code Structure
```text
cp_minigames_3.ino
├─ Utilities
│  ├─ pixelsOff(), solid(), setPix(), toneHz()
│  ├─ abResetHeld()          # debounced A+B reset with confirm blink
│  └─ waitButtonsReleased()  # release guard to avoid stale inputs
├─ splashAndPick()           # boot spinner + selection hints
├─ Reaction Timer
│  ├─ runReactionForever()
│  └─ runTiltMazeForever()   # alt-mode (press B while in Reaction)
├─ Rapid Fire
│  ├─ runRapidForever()
│  └─ runShakerForever()     # alt-mode (press B while in Rapid)
├─ Tug-of-War (default)
│  ├─ drawTugTrack()
│  ├─ runTugSolo()           # A from idle
│  ├─ runTugTwoPlayer()      # B from idle
│  └─ runTugForever()
└─ setup()/loop()            # boot pick → run selected game forever
```

## Troubleshooting

**No LEDs or tones after upload**  
- Check **board & port** in Arduino IDE.  
- Ensure **Adafruit Circuit Playground** library is installed.  
- Power-cycle the board. First run may not show Serial prints until the port enumerates.

**Rapid Fire feels unwinnable**  
- Increase `RF_DECAY_MS` (slows decay) and/or increase `RF_ROUND_MS`.  
- Aim for a steady mash; the bar decays at a fixed interval.

**Shaker Challenge doesn’t register motion**  
- Lower `SHK_THRESH` slightly (e.g., 1.0 → easier).  
- Hold the board in hand; table vibrations couple poorly.  
- Short, snappy shakes are better than slow arcs.

**Reaction Timer false-starts often**  
- Don’t rest fingers on buttons during the blue “armed” ring.  
- Widen the random delay by increasing `REACT_MAX_DELAY_MS`.

**Tug-of-War CPU too strong**  
- Make CPU slower (`CPU_PULSE_MS` ↑) and/or less aggressive (`CPU_PUSH_CHANCE` ↓).  
- For fair play, try Two-Player mode.

**A+B reset isn’t recognized**  
- Hold both buttons **continuously** until the purple confirm blink (~1.5s).  
- If your switches are finicky, bump `AB_RESET_MS` a little.

---

## FAQ

**Why doesn’t A+B return to the launcher?**  
For session stability. Use a hardware reset to re-select at boot.

**Can I change which game is default?**  
Yes. `splashAndPick()` maps “no buttons held” to **Tug-of-War**; feel free to modify.

**CPX vs CPB behavior?**  
Identical gameplay; audio loudness may differ slightly.

**Can Tug-of-War get an alt-mode too?**  
Sure—mirror the pattern used in Reaction/Rapid: check **B** while in Tug idle to branch.

---

## Credits
- **Design & Integration:** Iain Bennett  
- **Platform & Library:** Adafruit Circuit Playground