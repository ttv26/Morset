#include "audio.h"
#include "morse.h"
#include <cmath>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <cstdio>
    #include <cstdlib>
    #include <vector>
    #include <cstring>
#endif

namespace audio {

// PARIS timing: at 1 WPM, one dit = 1200ms
static int ditDurationMs(int wpm) {
    return 1200 / wpm;
}

static void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#ifdef _WIN32

void init() {
    // No initialization needed on Windows
}

static void beepTone(int freqHz, int durationMs) {
    Beep(static_cast<DWORD>(freqHz), static_cast<DWORD>(durationMs));
}

#else

void init() {
    // No initialization needed
}

// Generate a sine wave and pipe to aplay/paplay
static void beepTone(int freqHz, int durationMs) {
    const int sampleRate = 44100;
    const int numSamples = sampleRate * durationMs / 1000;
    const double amplitude = 0.8;

    std::vector<int16_t> samples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        samples[i] = static_cast<int16_t>(amplitude * 32767.0 *
            std::sin(2.0 * M_PI * freqHz * t));
    }

    // Try aplay first (ALSA), then paplay (PulseAudio)
    FILE* pipe = popen("aplay -q -f S16_LE -r 44100 -c 1 2>/dev/null", "w");
    if (!pipe) {
        pipe = popen("paplay --raw --format=s16le --rate=44100 --channels=1 2>/dev/null", "w");
    }
    if (pipe) {
        fwrite(samples.data(), sizeof(int16_t), numSamples, pipe);
        pclose(pipe);
    } else {
        // Fallback: terminal bell
        fprintf(stderr, "\a");
        sleepMs(durationMs);
    }
}

#endif

void playDit(int freqHz, int wpm) {
    int dit = ditDurationMs(wpm);
    beepTone(freqHz, dit);
}

void playDah(int freqHz, int wpm) {
    int dit = ditDurationMs(wpm);
    beepTone(freqHz, dit * 3);
}

void elementGap(int wpm) {
    sleepMs(ditDurationMs(wpm));
}

void charGap(int wpm) {
    sleepMs(ditDurationMs(wpm) * 3);
}

void wordGap(int wpm) {
    sleepMs(ditDurationMs(wpm) * 7);
}

void playMorseString(const std::string& morseCode, int freqHz, int wpm) {
    for (size_t i = 0; i < morseCode.size(); ++i) {
        char c = morseCode[i];
        if (c == '.') {
            playDit(freqHz, wpm);
        } else if (c == '-') {
            playDah(freqHz, wpm);
        } else if (c == ' ') {
            // Check for word separator " / "
            if (i + 2 < morseCode.size() && morseCode[i + 1] == '/' && morseCode[i + 2] == ' ') {
                wordGap(wpm);
                i += 2;
                continue;
            }
            charGap(wpm);
            continue;
        } else if (c == '/') {
            continue; // handled by space check
        }

        // Inter-element gap (between dits/dahs in same character)
        if (i + 1 < morseCode.size() && morseCode[i + 1] != ' ' && morseCode[i + 1] != '/') {
            elementGap(wpm);
        }
    }
}

void playChar(char c, int freqHz, int wpm) {
    std::string code = morse::encode(c);
    if (!code.empty()) {
        playMorseString(code, freqHz, wpm);
    }
}

void playText(const std::string& text, int freqHz, int wpm) {
    std::string morseStr = morse::encodeText(text);
    playMorseString(morseStr, freqHz, wpm);
}

} // namespace audio
