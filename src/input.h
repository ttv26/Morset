#pragma once
#include <string>

namespace input {

struct LineResult {
    std::string text;
    bool quit = false;  // true if Ctrl+Q was pressed
};

void init();
void cleanup();  // Restore terminal to normal mode

// Read a single character without Enter (raw mode).
// Returns: char value, 0x11=Ctrl+Q, 0x12=Ctrl+R, 27=Esc, -1=extended/ignored
int readChar();

// Read a full line (raw mode with echo + backspace support).
// Prints "  > prompt: " then reads until Enter or Ctrl+Q.
LineResult readLine(const std::string& prompt = "");

// Non-blocking: returns true if a key is waiting
bool hasKey();

// Wait for a Morse key press and measure hold duration.
//   keyCode : VK_ code on Windows, ASCII char value on Linux
//   ditMs   : one dit duration in milliseconds (1200 / wpm)
//   timeoutMs: -1 = wait forever, else returns '\0' on timeout
// Returns: '.' (dit), '-' (dah), '\0' (timeout), '\x11' (Ctrl+Q)
char waitMorseEvent(int keyCode, int ditMs, int timeoutMs = -1);

// Convert human-readable key name to platform key code.
// "SPACE"->32, "SHIFT"->VK_SHIFT(Win)/0(Linux), "A"->65, etc.
int  nameToCode(const std::string& name);
std::string codeToName(int code);

} // namespace input
