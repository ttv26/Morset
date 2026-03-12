#pragma once
#include <string>
#include <vector>

namespace ui {

// Enable ANSI colors on Windows terminal
void initTerminal();

// Clear the screen
void clearScreen();

// ANSI color helpers
std::string colorGreen(const std::string& text);
std::string colorRed(const std::string& text);
std::string colorYellow(const std::string& text);
std::string colorCyan(const std::string& text);
std::string colorBold(const std::string& text);
std::string colorDim(const std::string& text);
std::string colorMagenta(const std::string& text);

// Display the app banner
void showBanner();

// Display a menu and return the user's choice (1-based)
int showMenu(const std::string& title, const std::vector<std::string>& options);

// Prompt user for a string input
std::string promptString(const std::string& prompt);

// Prompt user for an integer input
int promptInt(const std::string& prompt, int minVal, int maxVal);

// Display a Morse code reference chart
void showMorseChart();

// Wait for user to press Enter
void pressEnterToContinue();

// Print a horizontal separator line
void printSeparator(int width = 60);

// Print a progress/loading bar
void printProgressBar(double fraction, int width = 40);

} // namespace ui
