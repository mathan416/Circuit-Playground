# CP_MiniGames_6 - Motion and Fitness Pack for Circuit Playground

`CP_MiniGames_6.ino` is a no-screen motion game pack for Adafruit Circuit Playground Express and Circuit Playground Bluefruit. It uses the NeoPixel ring, A/B buttons, accelerometer, and speaker.

## Games Included

- Step Sprint: default game when no button is held during boot.
- Freeze Dance: press B from Step Sprint idle.
- Balance Hold: hold A during boot.
- Jump Counter: hold B during boot.

## Hardware

- Adafruit Circuit Playground Express (SAMD21), or
- Adafruit Circuit Playground Bluefruit (nRF52840)

No external wiring is required.

## Library

Install the Adafruit Circuit Playground library. In this workspace it is available at:

```text
~/Developer/libraries/Adafruit_Circuit_Playground
```

## Upload

1. Open `CP_MiniGames_6/CP_MiniGames_6.ino` in Arduino IDE.
2. Select your Circuit Playground Express or Circuit Playground Bluefruit board.
3. Select the serial port for the board.
4. Upload.
5. Reset or power-cycle the board to use the boot picker.

## Boot Picker

| Boot input | Game |
| --- | --- |
| Hold A | Balance Hold |
| Hold B | Jump Counter |
| Hold nothing | Step Sprint |

The selected game keeps running until the board is reset.

## Universal Control

Hold A+B for about 1.5 seconds while inside a game to reset or exit that current mode. This does not return to the boot picker.

## Step Sprint

Step Sprint is a 15-second motion sprint.

### How to Play

1. Let Step Sprint start by holding no button during boot.
2. Press A to start the timer.
3. Move, step, or shake rhythmically to count steps.
4. The green meter fills as your count rises.
5. When time expires, the ring shows the ones digit of your score.

### Controls

| Input | Action |
| --- | --- |
| A from idle | Start sprint |
| Motion | Count steps |
| B from idle | Enter Freeze Dance |
| A+B hold | Reset Step Sprint |

## Freeze Dance

Freeze Dance is an alternate mode launched from Step Sprint.

### How to Play

1. Press B from Step Sprint idle.
2. Move while the ring is green.
3. Freeze while the ring is red.
4. Moving during red resets the round.
5. The round score is shown when the timer ends.

### Controls

| Input | Action |
| --- | --- |
| Motion during green | Score points |
| Stillness during red | Stay alive |
| A+B hold | Exit back to Step Sprint |

## Balance Hold

Balance Hold rewards keeping the board level.

### How to Play

1. Hold A during boot.
2. Hold the board as level as possible.
3. Each steady second fills another green LED.
4. Reach 10 seconds to win.

### Controls

| Input | Action |
| --- | --- |
| Keep level | Fill the meter |
| A+B hold | Reset Balance Hold |

## Jump Counter

Jump Counter counts strong motion spikes.

### How to Play

1. Hold B during boot.
2. Jump or make strong vertical motions.
3. Each detected jump increments the count.
4. The ring shows the ones digit; LED 9 also lights amber when the count reaches 10 or more.

### Controls

| Input | Action |
| --- | --- |
| Strong motion | Count a jump |
| A | Clear the count |
| A+B hold | Reset Jump Counter |

## Tuning Constants

Edit these near the top of `CP_MiniGames_6.ino`.

| Constant | Purpose |
| --- | --- |
| `BRIGHTNESS` | Overall NeoPixel brightness |
| `LAUNCHER_WINDOW_MS` | Boot picker selection time |
| `AB_RESET_MS` | A+B hold time |
| `SPRINT_MS` | Step Sprint round length |
| `FREEZE_ROUND_MS` | Freeze Dance round length |

## Troubleshooting

| Problem | Check |
| --- | --- |
| Steps count too easily | Raise the Step Sprint g-excess threshold |
| Steps do not count | Lower the Step Sprint g-excess threshold |
| Balance is too strict | Raise the tilt threshold in `runBalanceHoldForever()` |
| Jump Counter misses jumps | Lower the jump threshold in `runJumpCounterForever()` |

## Credits

Design and code: Iain Bennett and Codex  
Platform and library: Adafruit Circuit Playground
