# MorseT — Morse Code Trainer

A comprehensive, cross-platform CLI Morse code training application written in C++.

## Features

- **Text → Morse**: See a character, type its Morse code equivalent
- **Morse → Text**: See/hear Morse code, type the decoded character
- **Listen & Type**: Audio-only quiz — hear Morse beeps and identify the character
- **Custom Practice**: Choose specific characters to drill, or auto-target your weakest ones
- **Free Encode**: Type any text to see and hear its Morse code representation
- **Morse Reference Chart**: Full alphabet, numbers, and punctuation chart
- **Per-Letter Statistics**: Accuracy and error rate tracked for every character
- **Progress Graph**: ASCII bar chart showing accuracy trends across sessions
- **Configurable Speed**: Adjust WPM (5–50) and tone frequency (300–1200 Hz)
- **Persistent Stats**: Progress saved between sessions

## Building

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.15+

### Windows
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Linux
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run
```bash
# Windows
.\build\Release\morse_trainer.exe

# Linux
./build/morse_trainer
```

## Audio

- **Windows**: Uses the native `Beep()` API — no dependencies needed
- **Linux**: Generates PCM audio piped to `aplay` (ALSA) or `paplay` (PulseAudio)
  - Install ALSA utils: `sudo apt install alsa-utils`
  - Or PulseAudio utils: `sudo apt install pulseaudio-utils`

## Controls

| Key | Action |
|-----|--------|
| `q` | Quit current training session |
| `r` | Replay audio (in listen modes) |

## Morse Timing (PARIS standard)

| Element | Duration |
|---------|----------|
| Dit (.) | 1 unit |
| Dah (-) | 3 units |
| Intra-character gap | 1 unit |
| Inter-character gap | 3 units |
| Word gap | 7 units |

At 15 WPM: 1 unit = 80ms

## Data Storage

Stats are saved to:
- **Windows**: `%APPDATA%\MorseTrainer\stats.dat`
- **Linux**: `~/.morse_trainer/stats.dat`

##Built with AI

Claude Opus 4.6

## License

MIT
