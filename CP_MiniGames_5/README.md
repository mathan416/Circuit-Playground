# CP_MiniGames_5 - Puzzle and Logic Pack for Circuit Playground

`CP_MiniGames_5.ino` is a no-screen puzzle pack for Adafruit Circuit Playground Express and Circuit Playground Bluefruit. It uses the NeoPixel ring, A/B buttons, accelerometer, touch pads, and speaker.

## Games Included

- Lights Out: default game when no button is held during boot.
- Reflex Maze: press B from Lights Out.
- Code Breaker: hold A during boot.
- Safe Cracker: hold B during boot.

## Hardware

- Adafruit Circuit Playground Express (SAMD21), or
- Adafruit Circuit Playground Bluefruit (nRF52840)

No external wiring is required.

## Library

Install the Adafruit Circuit Playground library. 

## Upload

1. Open `CP_MiniGames_5/CP_MiniGames_5.ino` in Arduino IDE.
2. Select your Circuit Playground Express or Circuit Playground Bluefruit board.
3. Select the serial port for the board.
4. Upload.
5. Reset or power-cycle the board to use the boot picker.

## Boot Picker

| Boot input | Game |
| --- | --- |
| Hold A | Code Breaker |
| Hold B | Safe Cracker |
| Hold nothing | Lights Out |

The selected game keeps running until the board is reset.

## Universal Control

Hold A+B for about 1.5 seconds while inside a game to reset or exit that current mode. This does not return to the boot picker.

## Lights Out

Lights Out is a ring puzzle. Toggling one LED also toggles its two neighbors.

### How to Play

1. Let Lights Out start by holding no button during boot.
2. Tilt the board to move the blue cursor.
3. Press A to toggle the cursor LED and its neighbors.
4. Turn every lit LED off to win.

### Controls

| Input | Action |
| --- | --- |
| Tilt | Move the cursor |
| A | Toggle cursor and neighbors |
| B | Enter Reflex Maze |
| A+B hold | Reset Lights Out |

## Reflex Maze

Reflex Maze is an alternate mode launched from Lights Out. You tilt to a target and confirm before time expires.

### How to Play

1. Press B from Lights Out.
2. Tilt the blue cursor to the amber target.
3. Press A when the cursor is on the target.
4. Each success advances the route and shortens the timer.
5. A wrong press or timeout resets the route.

### Controls

| Input | Action |
| --- | --- |
| Tilt | Move the cursor |
| A | Confirm the current target |
| A+B hold | Exit back to Lights Out |

## Code Breaker

Code Breaker is a four-slot color code puzzle.

### How to Play

1. Hold A during boot.
2. Press A to cycle the color of the current slot.
3. Press B to move to the next slot.
4. Pressing B after the fourth slot submits the guess.
5. Green LEDs show exact matches.
6. Solve the code before eight guesses expire.

### Controls

| Input | Action |
| --- | --- |
| A | Cycle selected color |
| B | Advance slot or submit |
| A+B hold | Reset Code Breaker |

## Safe Cracker

Safe Cracker is a three-number tilt combination game.

### How to Play

1. Hold B during boot.
2. Tilt the board to move around the ring.
3. Blue means cold, amber means close, green means the correct number.
4. Press A when the cursor is green.
5. Crack all three numbers to open the safe.

### Controls

| Input | Action |
| --- | --- |
| Tilt | Move the dial |
| A | Try the current dial position |
| A+B hold | Reset Safe Cracker |

## Tuning Constants

Edit these near the top of `CP_MiniGames_5.ino`.

| Constant | Purpose |
| --- | --- |
| `BRIGHTNESS` | Overall NeoPixel brightness |
| `LAUNCHER_WINDOW_MS` | Boot picker selection time |
| `AB_RESET_MS` | A+B hold time |

## Troubleshooting

| Problem | Check |
| --- | --- |
| Tilt cursor feels backwards | Flip the board orientation or adjust `tiltIndex()` |
| Code Breaker feels too easy | Increase the number of slots in the code |
| Code Breaker feels too hard | Increase the guess limit in `runCodeBreakerForever()` |
| Safe Cracker gives too many hints | Remove the amber near-match hint |

## Credits

Design and code: Iain Bennett and Codex  
Platform and library: Adafruit Circuit Playground
