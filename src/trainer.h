#pragma once
#include "stats.h"
#include <vector>
#include <string>

namespace trainer {

struct TrainerConfig {
    int wpm = 15;            // Words per minute
    int frequency = 700;     // Tone frequency in Hz
    bool playAudio = true;   // Whether to play audio
    bool useMorseKey = false;        // Use tap/hold key for dit/dah input
    int  morseKeyCode = 32;          // Default: Space (32)
    std::string morseKeyName = "SPACE";
    std::vector<char> customChars;   // Empty = all chars
};

class Trainer {
public:
    Trainer(stats::StatsManager& statsManager);

    // Configure trainer settings
    void configure();

    // Training modes
    void textToMorse();       // User sees text, types Morse
    void morseToText();       // User sees/hears Morse, types text
    void listenAndType();     // Audio-only: user hears Morse, types letter
    void customPractice();    // User picks characters to drill
    void freeEncode();        // Free-form: type anything, see/hear Morse

    // Get current config
    const TrainerConfig& getConfig() const { return config_; }

private:
    stats::StatsManager& stats_;
    TrainerConfig config_;

    // Helper: get character set based on config
    std::vector<char> getCharSet() const;

    // Helper: run a single quiz round (Morse→Text / Listen).
    // showMorse=true displays the code, playSound=true plays audio.
    // Returns false when user wants to quit (Ctrl+Q).
    bool quizRound(char correctChar, bool showMorse, bool playSound);

    // Helper: accumulate dits/dahs via the configured Morse key.
    // Returns the Morse string (e.g. ".-") or "QUIT" on Ctrl+Q.
    std::string readMorseKeyInput();
};

} // namespace trainer
