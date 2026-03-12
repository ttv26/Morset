#pragma once
#include <string>

namespace audio {

// Initialize audio system
void init();

// Play a dit (short beep) at given frequency
void playDit(int freqHz, int wpm);

// Play a dah (long beep) at given frequency
void playDah(int freqHz, int wpm);

// Silence between elements within a character
void elementGap(int wpm);

// Silence between characters
void charGap(int wpm);

// Silence between words
void wordGap(int wpm);

// Play an entire Morse code string (e.g., ".- -...")
void playMorseString(const std::string& morseCode, int freqHz, int wpm);

// Play a single character as Morse audio
void playChar(char c, int freqHz, int wpm);

// Play a full text string as Morse audio
void playText(const std::string& text, int freqHz, int wpm);

} // namespace audio
