#include "trainer.h"
#include "morse.h"
#include "audio.h"
#include "ui.h"
#include "utils.h"
#include "input.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace trainer {

Trainer::Trainer(stats::StatsManager& statsManager) : stats_(statsManager) {}

// ─────────────────────────────────────────────────────────
//  Settings
// ─────────────────────────────────────────────────────────
void Trainer::configure() {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Trainer Settings ===")) << "\n\n";
    std::cout << "  WPM:             " << config_.wpm << "\n";
    std::cout << "  Frequency:       " << config_.frequency << " Hz\n";
    std::cout << "  Audio playback:  " << (config_.playAudio ? "ON" : "OFF") << "\n";
    std::cout << "  Morse key mode:  " << (config_.useMorseKey ? "ON" : "OFF") << "\n";
    std::cout << "  Morse key:       " << config_.morseKeyName << "\n\n";

    std::vector<std::string> opts = {
        "Change WPM (speed)",
        "Change tone frequency",
        "Toggle audio playback",
        "Morse key settings",
        "Back"
    };

    int choice = ui::showMenu("Settings", opts);
    switch (choice) {
        case 1:
            config_.wpm = ui::promptInt("Enter WPM", 5, 50);
            std::cout << ui::colorGreen("  WPM set to " + std::to_string(config_.wpm)) << "\n";
            ui::pressEnterToContinue();
            break;
        case 2:
            config_.frequency = ui::promptInt("Enter frequency (Hz)", 300, 1200);
            std::cout << ui::colorGreen("  Frequency set to " + std::to_string(config_.frequency) + " Hz") << "\n";
            ui::pressEnterToContinue();
            break;
        case 3:
            config_.playAudio = !config_.playAudio;
            std::cout << ui::colorGreen("  Audio " + std::string(config_.playAudio ? "enabled" : "disabled")) << "\n";
            ui::pressEnterToContinue();
            break;
        case 4: {
            ui::clearScreen();
            std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Morse Key Settings ===")) << "\n\n";
            std::cout << "  Mode:  " << (config_.useMorseKey ? ui::colorGreen("ON") : ui::colorDim("OFF")) << "\n";
            std::cout << "  Key:   " << config_.morseKeyName << "\n";
            std::cout << ui::colorDim("  Press tap = dit (.)  |  Press & hold = dah (-)") << "\n";
            std::cout << ui::colorDim("  Note: hold-detection works best on Windows or at WPM ≤ 8 on Linux.\n\n");

            std::vector<std::string> kopts = {
                std::string("Toggle Morse key mode (currently: ") + (config_.useMorseKey ? "ON" : "OFF") + ")",
                "Change Morse key (currently: " + config_.morseKeyName + ")",
                "Back"
            };
            int kc = ui::showMenu("Morse Key", kopts);
            if (kc == 1) {
                config_.useMorseKey = !config_.useMorseKey;
                std::cout << ui::colorGreen("  Morse key mode " +
                    std::string(config_.useMorseKey ? "ENABLED" : "DISABLED")) << "\n";
                ui::pressEnterToContinue();
            } else if (kc == 2) {
                std::cout << "\n  Supported: SPACE, A-Z, 0-9";
#ifdef _WIN32
                std::cout << ", SHIFT, ALT";
#else
                std::cout << " (SHIFT not available on Linux terminal)";
#endif
                std::cout << "\n";
                auto res = input::readLine("Enter key name");
                if (!res.quit && !res.text.empty()) {
                    config_.morseKeyName = utils::toUpper(utils::trim(res.text));
                    config_.morseKeyCode = input::nameToCode(config_.morseKeyName);
                    std::cout << ui::colorGreen("  Key set to: " + config_.morseKeyName) << "\n";
                    ui::pressEnterToContinue();
                }
            }
            break;
        }
        case 5: break;
    }
}

// ─────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────
std::vector<char> Trainer::getCharSet() const {
    if (!config_.customChars.empty()) return config_.customChars;
    return morse::getAllCharacters();
}

// Accumulate dits/dahs via the configured Morse key.
// Waits indefinitely for first press, then up to charGap for subsequent presses.
std::string Trainer::readMorseKeyInput() {
    int ditMs     = 1200 / config_.wpm;
    int charGapMs = ditMs * 3;
    std::string code;

    std::cout << "  " << ui::colorCyan("[") << config_.morseKeyName
              << ui::colorCyan("] tap=.  hold=-  Ctrl+Q=quit  → ");
    std::cout.flush();

    // First event: wait indefinitely
    char ev = input::waitMorseEvent(config_.morseKeyCode, ditMs, -1);
    if (ev == '\x11') { std::cout << "^Q\n"; return "QUIT"; }
    if (ev != '\0')   { code += ev; std::cout << ev; std::cout.flush(); }

    // Subsequent events: stop after inter-character silence
    while (true) {
        ev = input::waitMorseEvent(config_.morseKeyCode, ditMs, charGapMs);
        if (ev == '\x11') { std::cout << "^Q\n"; return "QUIT"; }
        if (ev == '\0')   break;   // inter-character timeout → done
        code += ev;
        std::cout << ev;
        std::cout.flush();
    }
    std::cout << "\n";
    return code;
}

// Single-character quiz round (used by morseToText and listenAndType).
// showMorse=true displays the Morse code on screen.
// Returns false when user presses Ctrl+Q.
bool Trainer::quizRound(char correctChar, bool showMorse, bool playSound) {
    std::string morseCode = morse::encode(correctChar);

    if (showMorse)
        std::cout << "\n  " << ui::colorCyan("Morse: ") << ui::colorBold(morseCode) << "\n";

    if (playSound && config_.playAudio) {
        std::cout << ui::colorDim("  [Playing...]") << "\n";
        audio::playChar(correctChar, config_.frequency, config_.wpm);
    }

    while (true) {
        std::cout << ui::colorGreen("  > ") << "Answer"
                  << ui::colorDim(" [Ctrl+Q=quit" +
                     std::string(playSound && config_.playAudio ? ", Ctrl+R=replay" : "") + "]: ");
        std::cout.flush();

        int c = input::readChar();

        if (c == 0x11) { std::cout << "^Q\n"; return false; }   // Ctrl+Q → quit

        if (c == 0x12 && playSound && config_.playAudio) {       // Ctrl+R → replay
            std::cout << "^R\n";
            audio::playChar(correctChar, config_.frequency, config_.wpm);
            continue;
        }

        if (c < 32) continue;   // other control chars, ignore

        char userChar = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        std::cout << userChar << "\n";

        bool correct = (userChar == correctChar);
        stats_.recordAttempt(correctChar, correct);

        if (correct) {
            std::cout << ui::colorGreen("  ✓ Correct! ")
                      << ui::colorBold(std::string(1, correctChar))
                      << " = " << morseCode << "\n";
        } else {
            std::cout << ui::colorRed("  ✗ Wrong!  ")
                      << "You: " << ui::colorRed(std::string(1, userChar))
                      << "   Correct: " << ui::colorGreen(ui::colorBold(std::string(1, correctChar)))
                      << ui::colorDim("  (" + morseCode + ")") << "\n";
        }
        return true;
    }
}

// ─────────────────────────────────────────────────────────
//  Training modes
// ─────────────────────────────────────────────────────────

// 1. Text → Morse
void Trainer::textToMorse() {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Text → Morse Training ===")) << "\n";
    if (config_.useMorseKey) {
        std::cout << ui::colorDim("  See a character.  Tap [") << config_.morseKeyName
                  << ui::colorDim("] = dit (.)  Hold = dah (-)  Ctrl+Q = quit\n");
    } else {
        std::cout << ui::colorDim("  See a character.  Type its Morse code (e.g. A = .-)\n");
        std::cout << ui::colorDim("  Ctrl+Q = quit\n");
    }

    auto charSet = getCharSet();
    int correct = 0, total = 0;

    while (true) {
        char   target      = utils::randomChar(charSet);
        std::string targetMorse = morse::encode(target);

        std::cout << "\n  " << ui::colorCyan("Character: ") << ui::colorBold(std::string(1, target));
        if (config_.playAudio) {
            std::cout << ui::colorDim("  [▶]");
            std::cout.flush();
            audio::playChar(target, config_.frequency, config_.wpm);
        }
        std::cout << "\n";

        std::string userInput;

        if (config_.useMorseKey) {
            userInput = readMorseKeyInput();
            if (userInput == "QUIT") break;
        } else {
            auto res = input::readLine("Type Morse code (Ctrl+Q=quit)");
            if (res.quit) break;
            userInput = utils::trim(res.text);
        }

        total++;
        bool ok = (userInput == targetMorse);
        stats_.recordAttempt(target, ok);

        if (ok) {
            correct++;
            std::cout << ui::colorGreen("  ✓ Correct! ")
                      << ui::colorBold(std::string(1, target)) << " = " << targetMorse << "\n";
        } else {
            std::cout << ui::colorRed("  ✗ Wrong!  ")
                      << "You: " << ui::colorRed(userInput)
                      << "   Correct: " << ui::colorGreen(ui::colorBold(std::string(1, target)))
                      << ui::colorDim("  = " + targetMorse) << "\n";
        }

        double acc = total > 0 ? static_cast<double>(correct) / total * 100.0 : 0.0;
        std::cout << ui::colorDim("  Score: " + std::to_string(correct) + "/" + std::to_string(total)
                  + "  (" + std::to_string(static_cast<int>(acc)) + "%)") << "\n";
    }

    input::cleanup();
    stats_.endSession();
    std::cout << "\n" << ui::colorCyan("  Session complete! ") << correct << "/" << total << " correct.\n";
    ui::pressEnterToContinue();
}

// 2. Morse → Text
void Trainer::morseToText() {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Morse → Text Training ===")) << "\n";
    std::cout << ui::colorDim("  See Morse code.  Press the matching letter.  Ctrl+Q=quit  Ctrl+R=replay\n");

    auto charSet = getCharSet();
    int correct = 0, total = 0;

    while (true) {
        char target = utils::randomChar(charSet);
        total++;
        if (!quizRound(target, true, true)) { total--; break; }

        // Recount correct from this session's stats
        correct = 0;
        for (auto& [ch, stat] : stats_.getAllCharStats()) correct += stat.correct;

        double acc = total > 0 ? static_cast<double>(correct) / total * 100.0 : 0.0;
        std::cout << ui::colorDim("  Score: " + std::to_string(correct) + "/" + std::to_string(total)
                  + "  (" + std::to_string(static_cast<int>(acc)) + "%)") << "\n";
    }

    input::cleanup();
    stats_.endSession();
    std::cout << "\n" << ui::colorCyan("  Session complete!") << "\n";
    ui::pressEnterToContinue();
}

// 3. Listen & Type
void Trainer::listenAndType() {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Listen & Type Training ===")) << "\n";
    std::cout << ui::colorDim("  Hear Morse code.  Press the matching letter.  Ctrl+Q=quit  Ctrl+R=replay\n");

    if (!config_.playAudio) {
        std::cout << ui::colorRed("\n  Audio is disabled! Enable it in Settings first.") << "\n";
        ui::pressEnterToContinue();
        return;
    }

    auto charSet = getCharSet();
    int correct = 0, total = 0;

    while (true) {
        char target = utils::randomChar(charSet);
        total++;
        if (!quizRound(target, false, true)) { total--; break; }

        correct = 0;
        for (auto& [ch, stat] : stats_.getAllCharStats()) correct += stat.correct;
        double acc = total > 0 ? static_cast<double>(correct) / total * 100.0 : 0.0;
        std::cout << ui::colorDim("  Score: " + std::to_string(correct) + "/" + std::to_string(total)
                  + "  (" + std::to_string(static_cast<int>(acc)) + "%)") << "\n";
    }

    input::cleanup();
    stats_.endSession();
    std::cout << "\n" << ui::colorCyan("  Session complete!") << "\n";
    ui::pressEnterToContinue();
}

// 4. Custom Practice — individual character picker
void Trainer::customPractice() {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Custom Practice ===")) << "\n\n";

    std::vector<std::string> opts = {
        "Letters (A-Z)  — pick individual or all",
        "Numbers (0-9)  — pick individual or all",
        "Letters + Numbers (full set)",
        "Any character  — type exactly which ones",
        "Practice weakest characters (auto-selected)",
        "Back"
    };

    int choice = ui::showMenu("Select character group", opts);

    auto pickFromSet = [&](const std::string& label,
                           char first, char last) -> bool {
        // Display available characters
        std::cout << "\n  " << ui::colorCyan(label + ": ");
        for (char c = first; c <= last; ++c) std::cout << c << " ";
        std::cout << "\n";
        std::cout << ui::colorDim("  Type the characters you want (e.g., ABCDE), or * for all:\n");

        auto res = input::readLine("Selection");
        if (res.quit) return false;

        std::string sel = utils::toUpper(utils::trim(res.text));
        config_.customChars.clear();

        if (sel == "*" || sel == "ALL") {
            for (char c = first; c <= last; ++c) config_.customChars.push_back(c);
        } else {
            for (char c : sel) {
                char uc = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                if (uc >= first && uc <= last) {
                    if (std::find(config_.customChars.begin(),
                                  config_.customChars.end(), uc) == config_.customChars.end())
                        config_.customChars.push_back(uc);
                }
            }
        }
        return !config_.customChars.empty();
    };

    switch (choice) {
        case 1:
            if (!pickFromSet("Letters", 'A', 'Z')) {
                std::cout << ui::colorRed("  No letters selected.\n");
                ui::pressEnterToContinue(); return;
            }
            break;
        case 2:
            if (!pickFromSet("Numbers", '0', '9')) {
                std::cout << ui::colorRed("  No numbers selected.\n");
                ui::pressEnterToContinue(); return;
            }
            break;
        case 3:
            config_.customChars.clear();
            for (char c = 'A'; c <= 'Z'; ++c) config_.customChars.push_back(c);
            for (char c = '0'; c <= '9'; ++c) config_.customChars.push_back(c);
            break;
        case 4: {
            std::cout << "\n  Available: ";
            for (char c : morse::getAllCharacters()) std::cout << c << " ";
            std::cout << "\n";
            auto res = input::readLine("Enter characters to practice");
            if (res.quit) return;
            std::string sel = utils::toUpper(utils::trim(res.text));
            config_.customChars.clear();
            for (char c : sel) {
                if (c != ' ' && morse::isSupported(c)) {
                    if (std::find(config_.customChars.begin(),
                                  config_.customChars.end(), c) == config_.customChars.end())
                        config_.customChars.push_back(c);
                }
            }
            if (config_.customChars.empty()) {
                std::cout << ui::colorRed("  No valid characters entered.\n");
                ui::pressEnterToContinue(); return;
            }
            break;
        }
        case 5: {
            auto weakest = stats_.getWeakestChars(10);
            if (weakest.empty()) {
                std::cout << ui::colorYellow("  Not enough data yet. Practice more first!\n");
                ui::pressEnterToContinue(); return;
            }
            config_.customChars.clear();
            std::cout << "\n  " << ui::colorCyan("Targeting: ");
            for (auto& [ch, stat] : weakest) {
                config_.customChars.push_back(ch);
                std::cout << ch << "(" << static_cast<int>(stat.accuracy()) << "%) ";
            }
            std::cout << "\n";
            break;
        }
        case 6: return;
    }

    if (choice >= 1 && choice <= 5) {
        std::cout << "\n  " << ui::colorGreen("Practicing: ");
        for (char c : config_.customChars) std::cout << c << " ";
        std::cout << "\n";

        std::vector<std::string> modeOpts = {
            "Morse → Text  (see & hear Morse, press letter)",
            "Text → Morse  (see letter, type/key Morse)",
            "Listen only   (hear Morse, press letter)"
        };
        int mode = ui::showMenu("Training mode", modeOpts);

        switch (mode) {
            case 1: morseToText();    break;
            case 2: textToMorse();    break;
            case 3: listenAndType();  break;
        }
        config_.customChars.clear();
    }
}

// 5. Free Encode — bidirectional (text→Morse or Morse→text)
void Trainer::freeEncode() {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Free Encode / Decode ===")) << "\n";
    std::cout << ui::colorDim("  Enter text  → shows & plays Morse.\n");
    std::cout << ui::colorDim("  Enter Morse (.- / ) → decodes to text.\n");
    std::cout << ui::colorDim("  Ctrl+Q = quit.\n\n");

    while (true) {
        auto res = input::readLine("Text or Morse");
        if (res.quit) break;

        std::string raw = utils::trim(res.text);
        if (raw.empty()) continue;

        // Detect Morse input: only dots, dashes, slashes, spaces
        bool isMorse = true;
        bool hasMorseChar = false;
        for (char c : raw) {
            if (c == '.' || c == '-') { hasMorseChar = true; }
            else if (c != '/' && c != ' ') { isMorse = false; break; }
        }

        if (isMorse && hasMorseChar) {
            // Morse → Text
            std::string decoded = morse::decodeText(raw);
            std::cout << "\n  " << ui::colorCyan("Morse: ") << raw << "\n";
            std::cout << "  " << ui::colorCyan("Text:  ") << ui::colorGreen(ui::colorBold(decoded)) << "\n\n";
        } else {
            // Text → Morse
            std::string upper   = utils::toUpper(raw);
            std::string encoded = morse::encodeText(upper);
            std::cout << "\n  " << ui::colorCyan("Text:  ") << upper << "\n";
            std::cout << "  " << ui::colorCyan("Morse: ") << ui::colorBold(encoded) << "\n";
            if (config_.playAudio) {
                std::cout << ui::colorDim("  [Playing...]") << "\n";
                audio::playText(upper, config_.frequency, config_.wpm);
            }
            std::cout << "\n";
        }
    }

    input::cleanup();
}

} // namespace trainer
