# CP_MiniGames_3 - Arcade Games for Circuit Playground

`CP_MiniGames_3.ino` is a no-screen arcade game pack for Adafruit Circuit Playground Express and Circuit Playground Bluefruit. It uses the 10 NeoPixels, A/B buttons, accelerometer, and speaker.

## Games Included

- Tug-of-War: default game when no button is held during boot.
- Reaction Timer: hold A during boot.
- Tilt Maze: press B from Reaction Timer idle.
- Rapid Fire: hold B during boot.
- Shaker Challenge: press B from Rapid Fire idle.

## Hardware

- Adafruit Circuit Playground Express (SAMD21), or
- Adafruit Circuit Playground Bluefruit (nRF52840)

No external wiring is required.

## Library

Install the Adafruit Circuit Playground library. 

The sketch includes:

```cpp
#include <Adafruit_CircuitPlayground.h>
```

## Upload

1. Open `CP_MiniGames_3/CP_MiniGames_3.ino` in Arduino IDE.
2. Select your Circuit Playground Express or Circuit Playground Bluefruit board.
3. Select the serial port for the board.
4. Upload.
5. Reset or power-cycle the board to use the boot picker.

## Boot Picker

The game is selected once, during the first 1.5 seconds after boot.

| Boot input | Game |
| --- | --- |
| Hold A | Reaction Timer |
| Hold B | Rapid Fire |
| Hold nothing | Tug-of-War |

The selected game keeps running until the board is reset.

## Universal Control

Hold A+B for about 1.5 seconds while inside a game to reset that game back to its own idle state. This does not return to the boot picker.

## Tug-of-War

Tug-of-War is the default game. It can be played solo against the CPU or as a two-player button-mashing match.

### How to Play

1. Let Tug-of-War start by holding no button during boot.
2. From idle, press A for solo mode or B for two-player mode.
3. In a match, each button press pulls the marker one step around the ring.
4. The left player tries to pull toward the left end.
5. The right player or CPU tries to pull toward the right end.
6. Reaching an end wins the match.

### Controls

| Input | Action |
| --- | --- |
| A from idle | Start solo vs CPU |
| B from idle | Start two-player |
| A during match | Pull left |
| B during match | Pull right |
| A+B hold | Reset to Tug-of-War idle |

### Solo Mode

- You use A.
- The CPU periodically attempts to push right.
- If the solo timer expires, the CPU wins.

## Reaction Timer

Reaction Timer measures how quickly you press a button after the GO signal.

### How to Play

1. Hold A during boot to enter Reaction Timer.
2. Press A from idle to arm the timer.
3. Wait while the ring shows the waiting state.
4. Do not press early.
5. When the board flashes green and plays the GO tone, press A or B as fast as possible.
6. Your reaction time is displayed on the LEDs.

### Controls

| Input | Action |
| --- | --- |
| A from idle | Arm the timer |
| A or B after GO | Stop the timer |
| B from idle | Enter Tilt Maze |
| A+B hold | Reset to Reaction Timer idle |

### False Start

Pressing A or B before GO triggers a red penalty flash and returns to idle.

### Score Display

- Amber flashes show hundreds of milliseconds.
- Green pips show tens of milliseconds.

## Tilt Maze

Tilt Maze is an alternate mode launched from Reaction Timer. You steer a blue ball through amber checkpoints around the ring.

### How to Play

1. Press B from Reaction Timer idle.
2. Tilt the board to move the blue ball.
3. Guide the ball to the amber checkpoint.
4. Hold near the checkpoint long enough to collect it.
5. Continue through the checkpoint path.
6. Drifting away too long causes a red failure flash and restarts the maze.

### Controls

| Input | Action |
| --- | --- |
| Tilt | Move the ball |
| A+B hold | Exit back to Reaction Timer |

## Rapid Fire

Rapid Fire is a timed button-mashing game.

### How to Play

1. Hold B during boot to enter Rapid Fire.
2. Press A from idle to start a 10-second heat.
3. Mash A to fill the green LED meter.
4. The meter decays while the timer runs.
5. Fill all 10 LEDs before time runs out to win.

### Controls

| Input | Action |
| --- | --- |
| A from idle | Start the heat |
| A during heat | Fill the meter |
| B from idle | Enter Shaker Challenge |
| A+B hold | Reset to Rapid Fire idle |

## Shaker Challenge

Shaker Challenge is an alternate mode launched from Rapid Fire. You shake the board to build and hold energy.

### How to Play

1. Press B from Rapid Fire idle.
2. Shake the board to build the teal meter.
3. Reach at least 9 LEDs.
4. Keep the meter at 9 or 10 LEDs for 1.5 seconds to win.
5. If the 12-second round timer expires first, you lose.

### Controls

| Input | Action |
| --- | --- |
| Shake | Build energy |
| A+B hold | Exit back to Rapid Fire |

### Notes

- Motion is measured as g-excess from the accelerometer.
- Small movement below `SHK_THRESH` does not add energy.
- Energy decays every tick, so steady motion matters.

## Tuning Constants

Edit these near the top of `CP_MiniGames_3.ino`.

| Constant | Purpose |
| --- | --- |
| `BRIGHTNESS` | Overall NeoPixel brightness |
| `LAUNCHER_WINDOW_MS` | Boot picker selection time |
| `AB_RESET_MS` | A+B hold time |
| `REACT_MIN_DELAY_MS`, `REACT_MAX_DELAY_MS` | Random wait before GO |
| `REACT_FALSE_PENALTY_MS` | Reaction false-start penalty |
| `TILT_STEP_MS` | Tilt Maze update cadence |
| `TILT_LOSE_MS` | Time allowed away from a checkpoint |
| `TILT_GAIN` | Tilt Maze sensitivity |
| `RF_ROUND_MS` | Rapid Fire round length |
| `RF_DECAY_MS` | Rapid Fire meter decay cadence |
| `RF_GOAL` | Rapid Fire target LEDs |
| `SHK_THRESH` | Shaker Challenge motion threshold |
| `SHK_EN_FROM_G` | Shaker energy gain |
| `SHK_EN_DECAY` | Shaker energy decay |
| `SHK_TICK_MS` | Shaker update cadence |
| `SHK_ROUND_MS` | Shaker round length |
| `SHK_HOLD_MS` | Time required near full meter |
| `TUG_TRACK_LEN` | Tug distance needed to win |
| `CPU_PULSE_MS`, `CPU_PUSH_CHANCE` | Solo CPU behavior |
| `SOLO_ROUND_MS` | Solo Tug-of-War timeout |

## Troubleshooting

| Problem | Check |
| --- | --- |
| Sketch will not compile | Confirm the Adafruit Circuit Playground library is installed |
| Upload fails | Confirm the selected board and port |
| Wrong game starts | Hold the boot-picker button before pressing reset |
| Reaction Timer false-starts | Release all buttons before arming |
| Rapid Fire is too hard | Increase `RF_ROUND_MS` or `RF_DECAY_MS` |
| Shaker Challenge is too hard | Lower `SHK_THRESH` or raise `SHK_EN_FROM_G` |
| Tug CPU is too strong | Lower `CPU_PUSH_CHANCE` or raise `CPU_PULSE_MS` |

## Credits

Design and code: Iain Bennett  
Platform and library: Adafruit Circuit Playground
