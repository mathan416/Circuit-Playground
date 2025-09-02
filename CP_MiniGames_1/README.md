# PlaygroundGames — CPX/CPB No-Screen Launcher + Mini Games

> Adafruit Circuit Playground **Express** (SAMD21) & **Bluefruit** (nRF52840)  
> One boot-time selector + five bite-size games, each tuned for the 10-LED NeoPixel ring, buttons, accelerometer, and capacitive pads.

- **Launcher (boot picker)** — choose once at power-up:
  - **A held** → **Simon-4** (touch pads A1/A2/A5/A6) → **Balance** (alt-mode via B)
  - **B held** → **Shake Dice** (shake to roll) → **Whack-a-Mole** (alt-mode via B)
  - **No buttons** → **Hot Potato** → (stand-alone game)
- **In-game universal:** **Hold A+B ≈1.5s** to reset the current game **to its own idle** (not the launcher).

## Table of Contents
- [Features](#features)
- [Hardware](#hardware)
- [Install & Build](#install--build)
- [Quick Start](#quick-start)
- [Launcher UX](#launcher-ux)
- [Games](#games)
  - [Hot Potato](#hot-potato)
  - [Shake Dice](#shake-dice)
    - [Alt-Mode: Whack-a-Mole](#alt-mode-whack-a-mole)
  - [Simon-4](#simon-4)
    - [Alt-Mode: Balance](#alt-mode-balance)
- [Tuning & Constants](#tuning--constants)
- [Code Structure](#code-structure)
- [Troubleshooting](#troubleshooting)
- [FAQ](#faq)
- [License](#license)
- [Credits](#credits)

---

## Features
- **No-screen** party games that use only the CPX/CPB **NeoPixel ring**, **buttons**, **accelerometer**, and **capacitive pads**.
- **Single boot selection**: pick a game in a ~1.5s window, then run it forever until a hardware reset.
- **Polished feedback**: breathing idles, confirm spinners, tones, and loss/win effects.
- **Alt-modes** accessed in-game (B button), so a single binary gives you multiple experiences.
- **Robust inputs**: debounced long-press reset (A+B), capacitive baseline calibration, hysteresis for releases.

---

## Hardware
- **Boards:**  
  - Adafruit **Circuit Playground Express** (SAMD21)  
  - Adafruit **Circuit Playground Bluefruit** (nRF52840)
- **Sensors used:**  
  - 10x NeoPixel ring  
  - Left/Right buttons (A/B)  
  - 3-axis accelerometer  
  - Capacitive pads (A1, A2, A5, A6)
- **Libraries:**  
  - **Adafruit Circuit Playground** (install via Arduino Library Manager)

No external wiring required.

---

## Install & Build
1. **Arduino IDE** → install board definitions for your CPX/CPB.
2. **Library Manager** → install **Adafruit Circuit Playground**.
3. Open `PlaygroundGames.ino`.
4. Select your board + port. **Upload**.
5. On reboot, you’ll see a **brief spinner/hints** window to choose a game.

---

## Quick Start
- **At boot (≈1.5s):**
  - Hold **A** → **Simon-4** (green spinner)
  - Hold **B** → **Shake Dice** (blue spinner)
  - Hold **none** → **Hot Potato** (amber default)
- **Any game:** hold **A+B ~1.5s** → reset to that game’s idle (confirmed by two blinks).

---

## Launcher UX
The launcher runs **once at boot**:
- If **A** is held → ring shows a **green** spinner.
- If **B** is held → **blue** spinner.
- If none → **amber** hint LEDs.
- After the window expires, the chosen game starts and **remains active** until hardware reset.

---

## Games

### Hot Potato
**Loop:** *Idle breathe* → *Round* → *Idle breathe* (repeat)

- **Idle:** Teal breathing animation. **Press A** to start a round.
- **Round:** A ticking cue accelerates; a hidden fuse determines the “boom.”  
  - **Motion penalty:** large shakes shorten the fuse slightly.  
  - **Right button** aborts round back to idle.
- **Lose:** Red flashes + low tones, then return to idle.

**Controls**
- **A** (idle) → start round  
- **Right button** (during round) → cancel to idle  
- **A+B (hold)** → reset to idle (confirmed by blink)

---

### Shake Dice
**Loop:** *Idle breathe* → *Detect shake or A* → *Spin* → *Show pips* → *Idle*

- **Idle:** Blue breathing.
- **Trigger:** **Shake** above threshold or **press A**.
- **Roll:** Short spin animation, then 1–6 pips on the ring + a tone.
- **Alt-mode:** **Press B** at any time to enter **Whack-a-Mole**.

**Controls**
- **Shake** or **A** → roll  
- **B** → enter Whack-a-Mole  
- **A+B (hold)** → reset to Dice idle

#### Alt-Mode: Whack-a-Mole
- **Goal:** Hit the lit LED (“mole”) **by shaking** (g-excess > threshold) **or press A** within a shrinking time window.
- **Round:** Timed (default 30s). Window narrows on success, widens after a miss.
- **Score Display:**  
  - **Tens** → amber ring flashes  
  - **Ones** → green pips (A+B exits during ones)

**Controls**
- **A** (idle) → start  
- **Shake or A** (during mole window) → hit  
- **A+B (hold)** → exit back to Dice

---

### Simon-4
**Pads:** A1 (red), A2 (green), A5 (blue), A6 (amber).  
**Idle:** Teal breathing; **press A** to start. Calibrates capacitive baselines every idle entry.  
**Gameplay:** Pattern playback → reproduce via touch pads with visual + tone feedback.  
**Alt-mode:** **Press B** in idle/wait states to enter **Balancer**.

**Controls**
- **A** (idle) → start Simon round  
- **Touch pads** (A1/A2/A5/A6) → input  
- **B** → enter Balancer  
- **A+B (hold)** → reset to Simon idle

#### Alt-Mode: Balance
- **Goal:** Keep the “ball” (blue LED) inside a moving amber **safe zone** on the ring by tilting the board.
- **Difficulty:** Zone shrinks over time; if it would shrink below 3 LEDs, you **win** (celebration, pause, restart).
- **Failure:** Staying outside the zone for too long triggers a fail flash, then zone resets a bit easier.

**Controls**
- **Tilt** to steer  
- **A** → re-center the safe zone to the current ball  
- **A+B (hold)** → exit back to Simon

---

## Tuning & Constants

You can adjust these at the top of `PlaygroundGames.ino`:

### Global
- `BRIGHTNESS` — NeoPixel brightness (0–255)
- `LAUNCHER_WINDOW_MS` — boot selection window (ms)
- `AB_RESET_MS` — A+B hold time to reset in-game (ms)

### Simon (capacitive)
- `CAP_SAMPLES` — samples to average for baseline calibration
- `CAP_DELTA_MIN` — required rise above baseline to count as touch
- `CAP_RELEASE_HYST` — margin below threshold to treat as released

### Dice / Shake
- `SHAKE_THRESH` — g-excess threshold to count as a shake

### Hot Potato
- `FUSE_MIN_S` / `FUSE_MAX_S` — random fuse range in seconds
- `TICK_START_S` / `TICK_MIN_S` / `TICK_DECAY` — ticking cadence curve

### Whack-a-Mole
- `START_WINDOW_MS` / `MIN_WINDOW_MS` / `DECAY` — hit-window difficulty curve
- `ROUND_MS` — round duration (ms)
- `HIT_SHAKE_GEX` — shake threshold (reuses/aligns with `SHAKE_THRESH`)

### Balancer
- `SAFE_WIDTH_START` / `SAFE_WIDTH_MIN` — zone half-width (total width = 2*W+1)
- `LOSE_MS` — allowed time outside the zone
- `STEP_MS` — update cadence (ms)
- `SHRINK_EVERY_MS` — zone shrink interval (ms)
- `WIN_PAUSE_MS` — pause after a win

---

## Code Structure

```text
PlaygroundGames.ino
├─ Utilities
│  ├─ pixelsOff(), solid(), toneHz()
│  ├─ abResetHeld()          # debounced long-press with confirm blink
│  └─ waitButtonsReleased()  # prevents stale inputs between phases
├─ splashAndPick()           # boot spinner + selection hints
├─ Hot Potato
│  ├─ potatoIdle()
│  ├─ potatoRound()
│  └─ runPotatoForever()
├─ Dice
│  ├─ runWhackAMoleForever() # alt-mode triggered from Dice via B
│  └─ runDiceForever()
├─ Simon
│  ├─ simonCalibrateCaps(), simonPadPressed(), simonWaitPadEdge()
│  ├─ flashPad(), loseFlash()
│  ├─ runBalancerForever()   # alt-mode triggered from Simon via B
│  └─ runSimonForever()
└─ setup()/loop()            # boot selection → run chosen game forever

---

## Troubleshooting

**No LEDs or tones after upload**  
- Verify **board & port** in Arduino IDE.  
- Ensure **Adafruit Circuit Playground** library is installed.  
- Power cycle. On some hosts, first run may hide Serial prints until enumerated.

**Capacitive pads feel “sticky” or too sensitive**  
- Increase `CAP_DELTA_MIN` (harder to trigger) or `CAP_RELEASE_HYST`.  
- Make sure you’re touching **A1/A2/A5/A6**, not the copper traces only.  
- Humidity or grounding can affect readings; try touching the CPX’s GND pad lightly.

**Dice rolls trigger accidentally**  
- Raise `SHAKE_THRESH` (more aggressive shake required).  
- Place the board on a stable surface; motion noise can accumulate.

**Balancer too easy/hard**  
- Change `SAFE_WIDTH_START`, `SAFE_WIDTH_MIN`, and `SHRINK_EVERY_MS`.  
- Adjust `LOSE_MS` if you’re frequently “clipping” out of the zone.

**A+B reset isn’t recognized**  
- Hold both buttons **continuously** until the confirm blink (≈1.5s).  
- If one button is flaky, try `waitButtonsReleased()` thresholds (rare).

---

## FAQ

**Why doesn’t A+B return to the launcher?**  
Consciously designed for **party stability**: once chosen, a game stays resident. This avoids accidental mode flips mid-play. Use hardware reset to re-select at boot.

**Can I change the boot selection time?**  
Yes—edit `LAUNCHER_WINDOW_MS`.

**Can I swap pad colors/frequencies for Simon?**  
Yes—edit the `PADS[]` table (pixel index, tone frequency, RGB, pin).

**Will this run identically on CPB and CPX?**  
Yes; behavior is identical. CPB audio volume may differ slightly.

**How do I make Hot Potato more frantic?**  
Lower `FUSE_MAX_S`, raise the motion penalty (shorten deadline more per g-excess), or increase tick acceleration with `TICK_DECAY`.


---

## Credits
- **Design & integration:** Iain Bennett
- **Platform & library:** Adafruit Circuit Playground

Enjoy! If you extend this (new alt-modes, new LED effects), consider a PR with a short demo clip or GIF.