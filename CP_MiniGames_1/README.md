# CP_MiniGames_1 - Party Games for Circuit Playground

`CP_MiniGames_1.ino` is a no-screen game pack for Adafruit Circuit Playground Express and Circuit Playground Bluefruit. It uses the 10 NeoPixels, A/B buttons, accelerometer, capacitive touch pads, and speaker.

## Games Included

- Hot Potato: default game when no button is held during boot.
- Simon-4: hold A during boot.
- Balancer: press B from Simon-4 idle/wait.
- Shake Dice: hold B during boot.
- Whack-a-Mole: press B from Shake Dice.

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

1. Open `CP_MiniGames_1/CP_MiniGames_1.ino` in Arduino IDE.
2. Select your Circuit Playground Express or Circuit Playground Bluefruit board.
3. Select the serial port for the board.
4. Upload.
5. Reset or power-cycle the board to use the boot picker.

## Boot Picker

The game is selected once, during the first 1.5 seconds after boot.

| Boot input | Game |
| --- | --- |
| Hold A | Simon-4 |
| Hold B | Shake Dice |
| Hold nothing | Hot Potato |

The selected game keeps running until the board is reset.

## Universal Control

Hold A+B for about 1.5 seconds while inside a game to reset that game back to its own idle state. This does not return to the boot picker.

## Hot Potato

Hot Potato is the default game. The board acts like a ticking potato with a hidden fuse.

### How to Play

1. Wait for the teal breathing idle animation.
2. Press A to start a round.
3. Pass or hold the board while it ticks.
4. The ticking speeds up until the hidden fuse explodes.
5. A red flash and low tone means the potato exploded.

### Controls

| Input | Action |
| --- | --- |
| A from idle | Start a round |
| B during a round | Cancel the round and return to idle |
| A+B hold | Reset to Hot Potato idle |

### Notes

- Strong motion can shorten the fuse slightly.
- The fuse time is randomized between `FUSE_MIN_S` and `FUSE_MAX_S`.

## Shake Dice

Shake Dice rolls a virtual die and shows the result as pips on the NeoPixel ring.

### How to Play

1. Hold B at boot to enter Shake Dice.
2. Shake the board or press A to roll.
3. Watch the spinner animation.
4. Count the lit pips for the result, 1 through 6.

### Controls

| Input | Action |
| --- | --- |
| Shake | Roll the die |
| A | Roll the die |
| B | Enter Whack-a-Mole |
| A+B hold | Reset to Shake Dice idle |

### Notes

- Shake sensitivity is controlled by `SHAKE_THRESH`.

## Whack-a-Mole

Whack-a-Mole is an alternate mode launched from Shake Dice. A mole appears as a lit LED, and you must hit it before the window expires.

### How to Play

1. Press B while in Shake Dice.
2. Press A from Whack-a-Mole idle to start.
3. When a mole LED appears, shake the board or press A to hit it.
4. Each success increases the score and tightens the timing.
5. Missing the mole briefly widens the timing window.
6. The round ends after the timer expires, then the score is displayed.

### Controls

| Input | Action |
| --- | --- |
| A from idle | Start the round |
| Shake during mole window | Hit the mole |
| A during mole window | Hit the mole |
| A+B hold | Exit back to Shake Dice |

### Score Display

- Amber flashes show the tens digit.
- Green pips show the ones digit.

## Simon-4

Simon-4 is a memory game using four capacitive touch pads.

### Pad Map

| Pad | Color |
| --- | --- |
| A1 | Red |
| A2 | Green |
| A5 | Blue |
| A6 | Amber |

### How to Play

1. Hold A at boot to enter Simon-4.
2. Wait for the teal idle animation.
3. Press A to start.
4. Watch and listen to the pattern.
5. Repeat the pattern by touching the matching pads.
6. The sequence grows as you succeed.
7. A wrong input triggers a loss flash and returns to idle.

### Controls

| Input | Action |
| --- | --- |
| A from idle | Start Simon-4 |
| A1/A2/A5/A6 | Enter the shown color |
| B from idle/wait | Enter Balancer |
| A+B hold | Reset to Simon-4 idle |

### Notes

- Capacitive baselines are recalibrated when Simon-4 enters idle.
- Touch sensitivity is controlled by `CAP_DELTA_MIN` and `CAP_RELEASE_HYST`.

## Balancer

Balancer is an alternate mode launched from Simon-4. You tilt the board to keep a blue ball inside an amber safe zone.

### How to Play

1. Press B from Simon-4 idle/wait.
2. Tilt the board to move the blue ball around the ring.
3. Keep the ball inside the amber safe zone.
4. The safe zone shrinks over time.
5. Staying outside the safe zone too long causes a failure flash.
6. Survive until the safe zone reaches its smallest target to win.

### Controls

| Input | Action |
| --- | --- |
| Tilt | Move the ball |
| A | Re-center the safe zone around the current ball position |
| A+B hold | Exit back to Simon-4 |

## Tuning Constants

Edit these near the top of `CP_MiniGames_1.ino`.

| Constant | Purpose |
| --- | --- |
| `BRIGHTNESS` | Overall NeoPixel brightness |
| `LAUNCHER_WINDOW_MS` | Boot picker selection time |
| `AB_RESET_MS` | A+B hold time |
| `SHAKE_THRESH` | Shake Dice roll sensitivity |
| `FUSE_MIN_S`, `FUSE_MAX_S` | Hot Potato fuse range |
| `TICK_START_S`, `TICK_MIN_S`, `TICK_DECAY` | Hot Potato tick timing |
| `CAP_SAMPLES`, `CAP_DELTA_MIN`, `CAP_RELEASE_HYST` | Simon touch behavior |

## Troubleshooting

| Problem | Check |
| --- | --- |
| Sketch will not compile | Confirm the Adafruit Circuit Playground library is installed |
| Upload fails | Confirm the selected board and port |
| Wrong game starts | Hold the boot-picker button before pressing reset |
| Touch pads misread | Let go of the pads while Simon-4 calibrates at idle |
| Shake games feel too hard | Lower the relevant shake threshold in the sketch |

## Credits

Design and code: Iain Bennett  
Platform and library: Adafruit Circuit Playground
