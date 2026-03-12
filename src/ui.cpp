#include "ui.h"
#include "input.h"
#include "morse.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <limits>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace ui {

void initTerminal() {
#ifdef _WIN32
    // Enable ANSI escape sequences on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    // Enable UTF-8 output
    SetConsoleOutputCP(CP_UTF8);
#endif
}

void clearScreen() {
    std::cout << "\033[2J\033[H" << std::flush;
}

std::string colorGreen(const std::string& text)   { return "\033[32m" + text + "\033[0m"; }
std::string colorRed(const std::string& text)      { return "\033[31m" + text + "\033[0m"; }
std::string colorYellow(const std::string& text)   { return "\033[33m" + text + "\033[0m"; }
std::string colorCyan(const std::string& text)     { return "\033[36m" + text + "\033[0m"; }
std::string colorBold(const std::string& text)     { return "\033[1m" + text + "\033[0m"; }
std::string colorDim(const std::string& text)      { return "\033[2m" + text + "\033[0m"; }
std::string colorMagenta(const std::string& text)  { return "\033[35m" + text + "\033[0m"; }

void showBanner() {
    std::cout << colorCyan(R"(
  __  __                    _____
 |  \/  | ___  _ __ ___  _|_   _|
 | |\/| |/ _ \| '__/ __|/ _ \| |
 | |  | | (_) | |  \__ \  __/| |
 |_|  |_|\___/|_|  |___/\___||_|
                      Morse Trainer
)") << std::endl;
    std::cout << colorDim("  A comprehensive Morse code training app") << std::endl;
    printSeparator();
}

int showMenu(const std::string& title, const std::vector<std::string>& options) {
    std::cout << "\n" << colorBold(colorYellow(title)) << "\n\n";
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << "  " << colorCyan("[" + std::to_string(i + 1) + "]") << " " << options[i] << "\n";
    }
    std::cout << std::endl;
    return promptInt("Your choice", 1, static_cast<int>(options.size()));
}

std::string promptString(const std::string& prompt) {
    auto res = input::readLine(prompt);
    return res.quit ? "" : res.text;
}

int promptInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        auto res = input::readLine(prompt + " (" + std::to_string(minVal) +
                                   "-" + std::to_string(maxVal) + ")");
        if (res.quit) return minVal;
        try {
            int val = std::stoi(res.text);
            if (val >= minVal && val <= maxVal) return val;
        } catch (...) {}
        std::cout << colorRed("  Invalid input. Please try again.") << "\n";
    }
}

void showMorseChart() {
    clearScreen();
    std::cout << "\n" << colorBold(colorYellow("  === Morse Code Reference Chart ===")) << "\n\n";

    auto& table = morse::getEncodeTable();
    std::vector<std::pair<char, std::string>> sorted(table.begin(), table.end());
    std::sort(sorted.begin(), sorted.end());

    // Letters
    std::cout << colorCyan("  Letters:") << "\n";
    int col = 0;
    for (auto& [ch, code] : sorted) {
        if (ch >= 'A' && ch <= 'Z') {
            std::cout << "    " << colorBold(std::string(1, ch)) << " "
                      << std::left << std::setw(8) << code;
            if (++col % 5 == 0) std::cout << "\n";
        }
    }
    if (col % 5 != 0) std::cout << "\n";

    // Numbers
    std::cout << "\n" << colorCyan("  Numbers:") << "\n";
    col = 0;
    for (auto& [ch, code] : sorted) {
        if (ch >= '0' && ch <= '9') {
            std::cout << "    " << colorBold(std::string(1, ch)) << " "
                      << std::left << std::setw(8) << code;
            if (++col % 5 == 0) std::cout << "\n";
        }
    }
    if (col % 5 != 0) std::cout << "\n";

    // Punctuation
    std::cout << "\n" << colorCyan("  Punctuation:") << "\n";
    col = 0;
    for (auto& [ch, code] : sorted) {
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))) {
            std::cout << "    " << colorBold(std::string(1, ch)) << " "
                      << std::left << std::setw(8) << code;
            if (++col % 4 == 0) std::cout << "\n";
        }
    }
    if (col % 4 != 0) std::cout << "\n";

    std::cout << "\n";
    printSeparator();
    std::cout << colorDim("  Timing: dit (.) = 1 unit, dah (-) = 3 units") << "\n";
    std::cout << colorDim("  Gap between elements = 1 unit, chars = 3 units, words = 7 units") << "\n\n";
    pressEnterToContinue();
}

void pressEnterToContinue() {
    std::cout << colorDim("  Press any key to continue...");
    std::cout.flush();
    input::readChar();
    input::cleanup();
    std::cout << "\n";
}

void printSeparator(int width) {
    std::cout << "  " << std::string(width, '-') << "\n";
}

void printProgressBar(double fraction, int width) {
    int filled = static_cast<int>(fraction * width);
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;

    std::cout << "  [";
    for (int i = 0; i < width; ++i) {
        if (i < filled) std::cout << colorGreen("█");
        else std::cout << colorDim("░");
    }
    std::cout << "] " << static_cast<int>(fraction * 100) << "%\n";
}

} // namespace ui
