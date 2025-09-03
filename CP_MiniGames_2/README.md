# PlaygroundGames — CPX/CPB No-Screen Fidget Pack (cp_minigames_2)

> Adafruit Circuit Playground **Express** (SAMD21) & **Bluefruit** (nRF52840)  
> One boot-time selector + three chill, tactile **fidgets** tuned for the 10-LED NeoPixel ring, buttons, and accelerometer.

- **Launcher (boot picker)** — choose once at power-up:
  - **A held** → **Orbit** (tilt-speed comet)
  - **B held** → **Twinkle** (motion-fed sparkles)
  - **No buttons** → **Spinner** (physicsy ring spinner) → **Comet Painter** (alt-mode via B)
- **In-game universal:** **Hold A+B ≈1.5 s** to reset the *current fidget* to its own idle (not the launcher).

## Table of Contents
- [Features](#features)
- [Hardware](#hardware)
- [Install & Build](#install--build)
- [Quick Start](#quick-start)
- [Launcher UX](#launcher-ux)
- [Fidgets](#fidgets)
  - [Spinner](#spinner)
    - [Alt-Mode: Comet Painter](#alt-mode-comet-painter)
  - [Orbit](#orbit)
  - [Twinkle](#twinkle)
- [Tuning & Constants](#tuning--constants)
- [Code Structure](#code-structure)
- [Troubleshooting](#troubleshooting)
- [FAQ](#faq)
- [License](#license)
- [Credits](#credits)

---

## Features
- **No-screen** pocket fidgets using only the CPX/CPB **NeoPixel ring**, **buttons**, and **accelerometer**.
- **Single boot selection**: pick a fidget in a ~1.4–1.5 s window; then it runs until a hardware reset.
- **Polished feedback**: breathing idles, confirm spinners, subtle tones, speed-tinted colors.
- **Alt-mode** inside Spinner (B button) for a second vibe without reflashing.
- **Robust inputs**: debounced long-press reset (A+B) with visual confirmation.

---

## Hardware
- **Boards:**  
  - Adafruit **Circuit Playground Express** (SAMD21)  
  - Adafruit **Circuit Playground Bluefruit** (nRF52840)
- **Sensors / I/O used:**  
  - 10× NeoPixel ring  
  - Left/Right buttons (A/B)  
  - 3-axis accelerometer
- **Libraries:**  
  - **Adafruit Circuit Playground** (install via Arduino Library Manager)

No external wiring required.

---

## Install & Build
1. **Arduino IDE** → install board definitions for CPX/CPB.
2. **Library Manager** → install **Adafruit Circuit Playground**.
3. Open `cp_minigames_2.ino`.
4. Select your board + port → **Upload**.
5. On reboot you’ll see a brief **hint/spinner** window for selection.

---

## Quick Start
- **At boot (≈1.4 s window):**
  - Hold **A** → **Orbit** (green spinner hint)
  - Hold **B** → **Twinkle** (blue spinner hint)
  - Hold **none** → **Spinner** (amber default)
- **Any fidget:** hold **A+B ~1.5 s** → reset to that fidget’s idle (two-blink confirm).

---

## Launcher UX
Runs **once at boot**:
- Holding **A** shows a **green** confirm spinner (Orbit).
- Holding **B** shows a **blue** confirm spinner (Twinkle).
- Holding none shows **amber** hints (Spinner).
- After the window expires, your choice **stays resident** until a hardware reset.

---

## Fidgets

### Spinner
A physics-y ring spinner. **Flicks/tilt** add momentum, **drag** gently eases it down. Color hue brightens with speed.

- **Loop:** measure tilt-derived angular velocity → integrate with drag → draw comet head + tail
- **Controls**
  - **A**: toggle *direction lock* (free / positive-only)
  - **B**: **enter Comet Painter** (alt-mode)
  - **A+B hold**: reset Spinner to idle

#### Alt-Mode: Comet Painter
Tilt to *steer* a colorful comet around the ring; adjustable tail & palette.

- **Controls**
  - **A**: cycle tail length (3 ↔ 5)
  - **B**: nudge hue anchor
  - **A+B hold**: exit back to Spinner

---

### Orbit
A calm orbiting comet whose **speed follows tilt magnitude**—more tilt, more flow.

- **Controls**
  - **A**: sensitivity (Low → Med → High)
  - **B**: dot size (1-LED ↔ 3-LED comet)
  - **A+B hold**: reset Orbit to idle

---

### Twinkle
Ambient **sparkles** that feed off **motion energy**; shake to “charge” denser twinkles. Pick a color family.

- **Controls**
  - **A**: density (Low → Med → High)
  - **B**: hue family (Rainbow → Warm → Cool)
  - **A+B hold**: reset Twinkle to idle

---

## Tuning & Constants
Edit these near the top of `cp_minigames_2.ino` to match your feel:

### Global
- `BRIGHTNESS` — NeoPixel brightness (0–255)  
- `LAUNCHER_WINDOW_MS` — boot selection window (ms)  
- `AB_RESET_MS` — A+B hold time for in-fidget reset (ms)

### Spinner physics
- `DT_S` — physics timestep (s)  
- `DRAG` — angular drag per step (0–1)  
- `KICK_GAIN` — how strongly tilt-derived angVel injects torque  
- `FLICK_THRESH` — rad/s to count as a flick impulse  
- `FLICK_BONUS` — extra omega on flick  
- `OMEGA_MAX` — clamp for stability

### Orbit (tilt → speed)
- `ORBIT_BASE` — base ω (rad/s)  
- `ORBIT_GAIN_LO/MD/HI` — sensitivities

### Twinkle (motion energy)
- `ENERGY_DECAY` — passive energy bleed (per step)  
- `ENERGY_FROM_SHAKE` — energy gained per g-excess  
- `DENS_LOW/MED/HI` — spawn density presets

---

## Code Structure
```text
cp_minigames_2.ino
├─ Utilities
│  ├─ pixelsOff(), solid(), wheel()
│  ├─ abResetHeld(), waitButtonsReleased()
│  ├─ ringAngle(), gex(), breatheOnce()
├─ Spinner
│  ├─ drawComet()
│  ├─ runCometPainterForever()      # alt-mode via B
│  └─ runSpinnerForever()
├─ Orbit
│  └─ runOrbitForever()
├─ Twinkle
│  └─ runTwinkleForever()
├─ Boot selection
│  └─ splashAndPick()               # A=Orbit, B=Twinkle, none=Spinner
└─ setup()/loop()                   # choose once, then run forever
```
---

## Troubleshooting

**Nothing happens after upload**  
- Verify **board & port** in Arduino IDE.  
- Ensure **Adafruit Circuit Playground** library is installed.  
- Power cycle the board; some hosts enumerate Serial late.

**Spinner feels sluggish / too wild**  
- For *more snap*: raise `KICK_GAIN`, lower `DRAG`, or increase `FLICK_BONUS`.  
- For *calmer*: lower `KICK_GAIN`, raise `DRAG`, or lower `OMEGA_MAX`.

**Orbit too sensitive**  
- Tap **A** in Orbit to cycle down sensitivity, or reduce `ORBIT_GAIN_*`.

**Twinkle never fills**  
- Move the board—motion feeds energy. Raise `ENERGY_FROM_SHAKE` or set higher density with **A**.

**A+B reset doesn’t trigger**  
- Hold both buttons until the double-blink (~1.5 s). Increase `AB_RESET_MS` if needed.

---

## FAQ

**Why doesn’t A+B return to the launcher?**  
For pocket-proofing: once selected, a fidget stays resident. Use a hardware reset to choose at boot again.

**Will it behave the same on CPX and CPB?**  
Yes; minor audio/IMU differences may change “feel” slightly—tweak constants to taste.

**Can I change colors/tails?**  
Yes—see `wheel()`, `drawComet()`, and in-mode controls (A/B).

---

## License
© 2025 Iain Bennett

---

## Credits
- **Design & code:** Iain Bennett  
- **Platform & library:** Adafruit Circuit Playground