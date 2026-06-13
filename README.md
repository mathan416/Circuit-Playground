# Circuit Playground Games

This repository contains three no-screen game packs for Adafruit Circuit Playground boards:

- Adafruit Circuit Playground Express (SAMD21)
- Adafruit Circuit Playground Bluefruit (nRF52840)

Each pack is a standalone Arduino sketch. Upload one sketch at a time, then use the boot picker on reset to choose the game or mode.

## Library

Install the Adafruit Circuit Playground library.


The sketches include:

```cpp
#include <Adafruit_CircuitPlayground.h>
```

## Packs

| Pack | Sketch | Games and modes |
| --- | --- | --- |
| Pack 1 | `CP_MiniGames_1/CP_MiniGames_1.ino` | Hot Potato, Simon-4, Balancer, Shake Dice, Whack-a-Mole |
| Pack 2 | `CP_MiniGames_2/CP_MiniGames_2.ino` | Spinner, Comet Painter, Orbit, Twinkle |
| Pack 3 | `CP_MiniGames_3/CP_MiniGames_3.ino` | Tug-of-War, Reaction Timer, Tilt Maze, Rapid Fire, Shaker Challenge |
| Pack 4 | `CP_MiniGames_4/CP_MiniGames_4.ino` | Beat Match, Metronome Toy, Tone Memory, Sound Meter |
| Pack 5 | `CP_MiniGames_5/CP_MiniGames_5.ino` | Lights Out, Reflex Maze, Code Breaker, Safe Cracker |
| Pack 6 | `CP_MiniGames_6/CP_MiniGames_6.ino` | Step Sprint, Freeze Dance, Balance Hold, Jump Counter |
| Pack 7 | `CP_MiniGames_7/CP_MiniGames_7.ino` | Duel, Memory Duel, Territory, Tug Deluxe |

## Quick Start

1. Open one of the `CP_MiniGames_*/*.ino` sketches in Arduino IDE.
2. Select your Circuit Playground Express or Circuit Playground Bluefruit board.
3. Select the serial port for the board.
4. Upload.
5. Reset or power-cycle the board.
6. Hold A, hold B, or hold nothing during boot to pick the game for that pack.

See the README inside each pack folder for the complete boot picker map, game instructions, controls, and tuning constants.

## Pack 1 Summary

See `CP_MiniGames_1/README.md`.

| Boot input | Game |
| --- | --- |
| Hold A | Simon-4 |
| Hold B | Shake Dice |
| Hold nothing | Hot Potato |

Alt-modes:

- Press B from Simon-4 idle/wait to enter Balancer.
- Press B from Shake Dice to enter Whack-a-Mole.

## Pack 2 Summary

See `CP_MiniGames_2/README.md`.

| Boot input | Fidget |
| --- | --- |
| Hold A | Orbit |
| Hold B | Twinkle |
| Hold nothing | Spinner |

Alt-mode:

- Press B from Spinner to enter Comet Painter.

## Pack 3 Summary

See `CP_MiniGames_3/README.md`.

| Boot input | Game |
| --- | --- |
| Hold A | Reaction Timer |
| Hold B | Rapid Fire |
| Hold nothing | Tug-of-War |

Alt-modes:

- Press B from Reaction Timer idle to enter Tilt Maze.
- Press B from Rapid Fire idle to enter Shaker Challenge.

## Pack 4 Summary

See `CP_MiniGames_4/README.md`.

| Boot input | Game |
| --- | --- |
| Hold A | Tone Memory |
| Hold B | Sound Meter |
| Hold nothing | Beat Match |

Alt-mode:

- Press B from Beat Match idle to enter Metronome Toy.

## Pack 5 Summary

See `CP_MiniGames_5/README.md`.

| Boot input | Game |
| --- | --- |
| Hold A | Code Breaker |
| Hold B | Safe Cracker |
| Hold nothing | Lights Out |

Alt-mode:

- Press B from Lights Out to enter Reflex Maze.

## Pack 6 Summary

See `CP_MiniGames_6/README.md`.

| Boot input | Game |
| --- | --- |
| Hold A | Balance Hold |
| Hold B | Jump Counter |
| Hold nothing | Step Sprint |

Alt-mode:

- Press B from Step Sprint idle to enter Freeze Dance.

## Pack 7 Summary

See `CP_MiniGames_7/README.md`.

| Boot input | Game |
| --- | --- |
| Hold A | Territory |
| Hold B | Tug Deluxe |
| Hold nothing | Duel |

Alt-mode:

- Press B from Duel idle to enter Memory Duel.

## Universal Control

In each pack, hold A+B for about 1.5 seconds while inside a game or fidget to reset that current mode back to its own idle state. This does not return to the boot picker; reset the board to choose a different boot game.

## Credits

Design and code: Iain Bennett  
Platform and library: Adafruit Circuit Playground
