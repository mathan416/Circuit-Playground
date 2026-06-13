# CP_MiniGames_4 - Rhythm and Sound Pack for Circuit Playground

`CP_MiniGames_4.ino` is a no-screen rhythm and sound pack for Adafruit Circuit Playground Express and Circuit Playground Bluefruit. It uses the NeoPixel ring, A/B buttons, speaker, and sound sensor.

## Games Included

- Beat Match: default game when no button is held during boot.
- Metronome Toy: press B from Beat Match idle.
- Tone Memory: hold A during boot.
- Sound Meter: hold B during boot.

## Hardware

- Adafruit Circuit Playground Express (SAMD21), or
- Adafruit Circuit Playground Bluefruit (nRF52840)

No external wiring is required.

## Library

Install the Adafruit Circuit Playground library. 

## Upload

1. Open `CP_MiniGames_4/CP_MiniGames_4.ino` in Arduino IDE.
2. Select your Circuit Playground Express or Circuit Playground Bluefruit board.
3. Select the serial port for the board.
4. Upload.
5. Reset or power-cycle the board to use the boot picker.

## Boot Picker

| Boot input | Game |
| --- | --- |
| Hold A | Tone Memory |
| Hold B | Sound Meter |
| Hold nothing | Beat Match |

The selected game keeps running until the board is reset.

## Universal Control

Hold A+B for about 1.5 seconds while inside a game to reset or exit that current mode. This does not return to the boot picker.

## Beat Match

Beat Match asks you to tap along with a steady pulse.

### How to Play

1. Let Beat Match start by holding no button during boot.
2. Press A as close to each beat as possible.
3. Good taps flash green and play a higher tone.
4. Missed taps flash red.
5. After 16 beats, the ring shows your score.

### Controls

| Input | Action |
| --- | --- |
| A | Tap the beat |
| B from idle | Enter Metronome Toy |
| A+B hold | Reset Beat Match |

## Metronome Toy

Metronome Toy is an alternate mode launched from Beat Match. It plays a repeating 4-beat pattern.

### Controls

| Input | Action |
| --- | --- |
| A | Cycle tempo from slow to fast |
| B | Cycle accent pattern |
| A+B hold | Exit back to Beat Match |

## Tone Memory

Tone Memory is a Simon-style pitch game using two buttons and two touch pads.

### Inputs

| Input | Tone/color slot |
| --- | --- |
| A | Slot 1 |
| B | Slot 2 |
| Touch A1 | Slot 3 |
| Touch A6 | Slot 4 |

### How to Play

1. Hold A during boot.
2. Watch and listen to the sequence.
3. Repeat it with A, B, A1, and A6.
4. The sequence grows after each success.
5. A wrong input resets the game to level 1.

## Sound Meter

Sound Meter turns the ring into a simple sound level display.

### Controls

| Input | Action |
| --- | --- |
| Speak, clap, or play sound | Fill the meter |
| A | Recalibrate the quiet baseline |
| B | Cycle color palette |
| A+B hold | Reset Sound Meter |

## Tuning Constants

Edit these near the top of `CP_MiniGames_4.ino`.

| Constant | Purpose |
| --- | --- |
| `BRIGHTNESS` | Overall NeoPixel brightness |
| `LAUNCHER_WINDOW_MS` | Boot picker selection time |
| `AB_RESET_MS` | A+B hold time |

## Troubleshooting

| Problem | Check |
| --- | --- |
| Beat Match feels strict | Increase the timing window in `runBeatForever()` |
| Tone Memory touch inputs are unreliable | Try touching A1/A6 more firmly or adjust the readCap threshold |
| Sound Meter is always full | Press A in a quiet room to recalibrate |
| Sound Meter barely moves | Lower the level divisor in `runSoundMeterForever()` |

## Credits

Design and code: Iain Bennett and Codex  
Platform and library: Adafruit Circuit Playground
