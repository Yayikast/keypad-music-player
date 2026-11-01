# Keypad Music Player

A small music player built on an Arduino Uno. A 4x4 matrix keypad picks one of four built-in songs or plays single notes. A passive buzzer plays the melody, and seven LEDs (one for each note from C4 to B4) light up with each note.

Playback is **non-blocking**: note and gap timing use `millis()` rather than `delay()`, so the keypad stays responsive while a song plays (pause, skip and speed changes take effect right away).

*Mini project for the Microcontroller course at SIIT (Sirindhorn International Institute of Technology), Year 2, Term 2. Written in Arduino C/C++ for the Arduino Uno (ATmega328P).*

## Features

- 4 songs: *Twinkle Twinkle Little Star*, *Happy Birthday*, *Jingle Bells*, *Ode to Joy*
- Play/pause, next/previous song
- Tempo control (slower / faster) plus a hardware reset button
- "Piano mode": play any single note from the keypad
- Status messages over Serial (9600 baud)

## Controls

| Key | Action |
|---|---|
| `A` `B` `C` `D` | Play song A–D from the start |
| `1`–`7` | Play a single note (C, D, E, F, G, A, B); stops the current song |
| `0` | Pause / resume |
| `#` | Next song |
| `*` | Previous song |
| `8` | Slower (note length ×1.5, max 2000 ms) |
| `9` | Faster (note length ÷1.5, min 20 ms) |
| Reset button | Reset tempo to 400 ms per note |

## Hardware

- Arduino Uno
- 4x4 membrane keypad
- Passive buzzer
- 7 LEDs + 7 current-limiting resistors (220–330 Ω)
- 1 push button

## Wiring

| Component | Arduino pin |
|---|---|
| Keypad rows R1–R4 | `A0`, `A1`, `A2`, `A3` |
| Keypad columns C1–C4 | `A4`, `A5`, `10`, `11` |
| Buzzer (+) | `9` |
| LEDs C, D, E, F, G, A, B | `2`, `3`, `4`, `5`, `6`, `7`, `8` (through a resistor to GND) |
| Reset button | `12` → GND (uses the internal pull-up) |

## Dependencies

- [Keypad](https://github.com/Chris--A/Keypad) library by Mark Stanley & Alexander Brevig (Arduino Library Manager: "Keypad")

## Building

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) and the Keypad library (Library Manager → search "Keypad").
2. Clone this repo into a folder named `keypad_music_player`, since the Arduino IDE requires the folder name to match the `.ino` file.
3. Open `keypad_music_player.ino`, select **Board: Arduino Uno**, then Verify / Upload.

With `arduino-cli`:

```bash
arduino-cli compile --fqbn arduino:avr:uno keypad_music_player
```

## How it works

Each song is stored as an array of note indices (0 = C … 6 = B), which look up both the buzzer frequency and the LED to light. `playSong()` is a small state machine that runs on every `loop()` pass. It starts a note, holds it for 85% of the beat, then rests for the other 15% before moving to the next note, which keeps repeated notes distinct. Because the buzzer only covers one octave (C4–B4), *Happy Birthday*'s octave jump is approximated.

## License

Released under the [MIT License](LICENSE). The Keypad library is a separate project with its own license.
