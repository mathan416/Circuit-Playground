# CP_MiniGames_2 - Fidget Pack for Circuit Playground

`CP_MiniGames_2.ino` is a no-screen fidget pack for Adafruit Circuit Playground Express and Circuit Playground Bluefruit. It uses the 10 NeoPixels, A/B buttons, accelerometer, and speaker.

## Fidgets Included

- Spinner: default mode when no button is held during boot.
- Comet Painter: press B from Spinner.
- Orbit: hold A during boot.
- Twinkle: hold B during boot.

## Hardware

- Adafruit Circuit Playground Express (SAMD21), or
- Adafruit Circuit Playground Bluefruit (nRF52840)

No external wiring is required.

## Library

Install the Adafruit Circuit Playground library. In this workspace it is available at:

```text
~/Developer/libraries/Adafruit_Circuit_Playground
```

The sketch includes:

```cpp
#include <Adafruit_CircuitPlayground.h>
```

## Upload

1. Open `CP_MiniGames_2/CP_MiniGames_2.ino` in Arduino IDE.
2. Select your Circuit Playground Express or Circuit Playground Bluefruit board.
3. Select the serial port for the board.
4. Upload.
5. Reset or power-cycle the board to use the boot picker.

## Boot Picker

The fidget is selected once, during the first 1.4 seconds after boot.

| Boot input | Fidget |
| --- | --- |
| Hold A | Orbit |
| Hold B | Twinkle |
| Hold nothing | Spinner |

The selected fidget keeps running until the board is reset.

## Universal Control

Hold A+B for about 1.5 seconds while inside a fidget to reset that fidget back to its own idle state. This does not return to the boot picker.

## Spinner

Spinner is a physics-style ring spinner. Tilt and flick motion add momentum, and drag gradually slows the comet.

### How to Use

1. Let the amber default mode start after boot, or hold nothing during the boot picker.
2. Tilt or flick the board to add spin.
3. Watch the comet head and tail move around the ring.
4. The color shifts with motion and speed.

### Controls

| Input | Action |
| --- | --- |
| Tilt/flick | Add momentum |
| A | Toggle direction lock |
| B | Enter Comet Painter |
| A+B hold | Reset Spinner |

### Direction Lock

Pressing A toggles whether the spinner can move freely in either direction or is biased to the positive direction only.

## Comet Painter

Comet Painter is an alternate mode launched from Spinner. It turns the ring into a tilt-controlled comet with adjustable tail length and hue.

### How to Use

1. Press B while in Spinner.
2. Tilt the board to steer the comet around the ring.
3. Press A to change the tail length.
4. Press B to shift the hue anchor.

### Controls

| Input | Action |
| --- | --- |
| Tilt | Move the comet |
| A | Toggle tail length |
| B | Nudge the hue anchor |
| A+B hold | Exit back to Spinner |

## Orbit

Orbit is a calm orbiting comet whose speed follows the tilt amount.

### How to Use

1. Hold A during boot to enter Orbit.
2. Tilt the board gently for slow motion.
3. Tilt farther for faster orbiting.
4. Use A and B to tune the feel while it runs.

### Controls

| Input | Action |
| --- | --- |
| Tilt | Control orbit speed |
| A | Cycle sensitivity: low, medium, high |
| B | Toggle comet size: 1 LED or 3 LEDs |
| A+B hold | Reset Orbit |

## Twinkle

Twinkle creates motion-fed sparkles. Movement charges the effect, and the LEDs fade down over time.

### How to Use

1. Hold B during boot to enter Twinkle.
2. Move or shake the board to feed sparkle energy.
3. Leave it still for a calmer pattern.
4. Use A and B to change density and color family.

### Controls

| Input | Action |
| --- | --- |
| Move/shake | Add sparkle energy |
| A | Cycle density: low, medium, high |
| B | Cycle hue family: rainbow, warm, cool |
| A+B hold | Reset Twinkle |

## Tuning Constants

Edit these near the top of `CP_MiniGames_2.ino`.

| Constant | Purpose |
| --- | --- |
| `BRIGHTNESS` | Overall NeoPixel brightness |
| `LAUNCHER_WINDOW_MS` | Boot picker selection time |
| `AB_RESET_MS` | A+B hold time |
| `DT_S` | Spinner physics timestep |
| `DRAG` | Spinner slowdown per step |
| `KICK_GAIN` | How strongly tilt adds spin |
| `FLICK_THRESH` | Motion threshold for a flick |
| `FLICK_BONUS` | Extra spin added by a flick |
| `OMEGA_MAX` | Maximum spinner speed |
| `ORBIT_BASE` | Base Orbit speed |
| `ORBIT_GAIN_LO`, `ORBIT_GAIN_MD`, `ORBIT_GAIN_HI` | Orbit sensitivity presets |
| `ENERGY_DECAY` | Twinkle energy decay |
| `ENERGY_FROM_SHAKE` | Twinkle energy gained from motion |
| `DENS_LOW`, `DENS_MED`, `DENS_HI` | Twinkle density presets |

## Troubleshooting

| Problem | Check |
| --- | --- |
| Sketch will not compile | Confirm the Adafruit Circuit Playground library is installed |
| Upload fails | Confirm the selected board and port |
| Wrong fidget starts | Hold the boot-picker button before pressing reset |
| Spinner feels too slow | Raise `KICK_GAIN`, lower `DRAG`, or raise `FLICK_BONUS` |
| Spinner feels too wild | Lower `KICK_GAIN` or `OMEGA_MAX`, or raise `DRAG` |
| Twinkle is too sparse | Increase density with A or raise `ENERGY_FROM_SHAKE` |

## Credits

Design and code: Iain Bennett  
Platform and library: Adafruit Circuit Playground
