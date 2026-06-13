# CP_MiniGames_7 - Two-Player Pack for Circuit Playground

`CP_MiniGames_7.ino` is a no-screen two-player pack for Adafruit Circuit Playground Express and Circuit Playground Bluefruit. It uses the NeoPixel ring, A/B buttons, and speaker.

## Games Included

- Duel: default game when no button is held during boot.
- Memory Duel: press B from Duel idle.
- Territory: hold A during boot.
- Tug Deluxe: hold B during boot.

## Hardware

- Adafruit Circuit Playground Express (SAMD21), or
- Adafruit Circuit Playground Bluefruit (nRF52840)

No external wiring is required.

## Library

Install the Adafruit Circuit Playground library. 

## Upload

1. Open `CP_MiniGames_7/CP_MiniGames_7.ino` in Arduino IDE.
2. Select your Circuit Playground Express or Circuit Playground Bluefruit board.
3. Select the serial port for the board.
4. Upload.
5. Reset or power-cycle the board to use the boot picker.

## Boot Picker

| Boot input | Game |
| --- | --- |
| Hold A | Territory |
| Hold B | Tug Deluxe |
| Hold nothing | Duel |

The selected game keeps running until the board is reset.

## Universal Control

Hold A+B for about 1.5 seconds while inside a game to reset or exit that current mode. This does not return to the boot picker.

## Duel

Duel is a fastest-finger reaction game.

### How to Play

1. Let Duel start by holding no button during boot.
2. Release both buttons.
3. Wait through the blue ready state.
4. When the ring turns green, the first player to press wins.
5. A is the left player. B is the right player.
6. Pressing early awards the win to the other player.

### Controls

| Input | Action |
| --- | --- |
| A after green | Left player wins |
| B after green | Right player wins |
| B from idle | Enter Memory Duel |
| A+B hold | Reset Duel |

## Memory Duel

Memory Duel is an alternate mode launched from Duel. Players alternate repeating a left/right sequence.

### How to Play

1. Press B from Duel idle.
2. Watch the left/right sequence.
3. The active player repeats it with A and B.
4. A mistake gives the round to the other player.
5. After each success, the sequence grows and turns alternate.

### Controls

| Input | Action |
| --- | --- |
| A | Left input |
| B | Right input |
| A+B hold | Exit back to Duel |

## Territory

Territory is a timed area-control button game.

### How to Play

1. Hold A during boot.
2. Player A claims LEDs moving clockwise.
3. Player B claims LEDs moving counter-clockwise.
4. After 30 seconds, the player with more claimed LEDs wins.

### Controls

| Input | Action |
| --- | --- |
| A | Left player claims next LED |
| B | Right player claims next LED |
| A+B hold | Reset Territory |

## Tug Deluxe

Tug Deluxe is a faster tug-of-war variant with automatic drift back toward center.

### How to Play

1. Hold B during boot.
2. Mash A to pull left.
3. Mash B to pull right.
4. The marker drifts toward center over time.
5. Pull the marker to your end, or be ahead when the timer expires.

### Controls

| Input | Action |
| --- | --- |
| A | Pull left |
| B | Pull right |
| A+B hold | Reset Tug Deluxe |

## Tuning Constants

Edit these near the top of `CP_MiniGames_7.ino`.

| Constant | Purpose |
| --- | --- |
| `BRIGHTNESS` | Overall NeoPixel brightness |
| `LAUNCHER_WINDOW_MS` | Boot picker selection time |
| `AB_RESET_MS` | A+B hold time |

## Troubleshooting

| Problem | Check |
| --- | --- |
| Duel false-starts often | Release both buttons before the blue ready state |
| Memory Duel is too fast | Increase playback delays in `runMemoryDuelForever()` |
| Territory feels too long | Shorten the 30-second timer in `runTerritoryForever()` |
| Tug Deluxe is too centered | Increase the decay interval or reduce the decay amount |

## Credits

Design and code: Iain Bennett and Codex  
Platform and library: Adafruit Circuit Playground
