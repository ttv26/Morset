#include "input.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <thread>
#include <iostream>

// ─────────────────────────────────────────────────────────
//  WINDOWS
// ─────────────────────────────────────────────────────────
#ifdef _WIN32
#include <conio.h>
#include <windows.h>

namespace input {

void init()    {}
void cleanup() {}

int readChar() {
    int c = _getch();
    if (c == 0 || c == 0xE0) { _getch(); return -1; }  // extended key
    if (c == '\r') return '\n';
    return c;
}

LineResult readLine(const std::string& prompt) {
    if (!prompt.empty())
        std::cout << "\033[32m  > \033[0m" << prompt << ": ";
    std::cout.flush();

    std::string result;
    while (true) {
        int c = readChar();
        if (c == 0x11) { std::cout << "\n"; return {"", true}; }
        if (c == '\n') { std::cout << "\n"; return {result, false}; }
        if (c == 8 || c == 127) {
            if (!result.empty()) {
                result.pop_back();
                std::cout << "\b \b";
                std::cout.flush();
            }
            continue;
        }
        if (c >= 32 && c < 127) {
            result += static_cast<char>(c);
            std::cout << static_cast<char>(c);
            std::cout.flush();
        }
    }
}

bool hasKey() { return _kbhit() != 0; }

char waitMorseEvent(int vkCode, int ditMs, int timeoutMs) {
    using clk = std::chrono::steady_clock;

    auto deadline = (timeoutMs >= 0)
        ? clk::now() + std::chrono::milliseconds(timeoutMs)
        : clk::time_point::max();

    auto ctrlQ = [&]() -> bool {
        return (GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
               (GetAsyncKeyState('Q')        & 0x8000);
    };

    // Wait for key press or timeout
    while (!(GetAsyncKeyState(vkCode) & 0x8000)) {
        if (ctrlQ()) return '\x11';
        if (clk::now() >= deadline) return '\0';
        Sleep(2);
    }

    auto t0 = clk::now();

    // Wait for key release
    while (GetAsyncKeyState(vkCode) & 0x8000) {
        if (ctrlQ()) return '\x11';
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            clk::now() - t0).count();
        if (elapsed > 10000) break;  // safety cap
        Sleep(2);
    }

    int ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            clk::now() - t0).count());

    return (ms < ditMs * 2) ? '.' : '-';
}

int nameToCode(const std::string& name) {
    std::string up = name;
    std::transform(up.begin(), up.end(), up.begin(), ::toupper);
    if (up == "SPACE") return VK_SPACE;
    if (up == "SHIFT") return VK_SHIFT;
    if (up == "ALT")   return VK_MENU;
    if (up == "CTRL")  return VK_CONTROL;
    if (up.size() == 1 && std::isalnum(static_cast<unsigned char>(up[0])))
        return static_cast<int>(up[0]);
    return VK_SPACE;
}

std::string codeToName(int code) {
    if (code == VK_SPACE)   return "SPACE";
    if (code == VK_SHIFT)   return "SHIFT";
    if (code == VK_MENU)    return "ALT";
    if (code == VK_CONTROL) return "CTRL";
    if (code >= 'A' && code <= 'Z') return std::string(1, static_cast<char>(code));
    if (code >= '0' && code <= '9') return std::string(1, static_cast<char>(code));
    return "SPACE";
}

} // namespace input

// ─────────────────────────────────────────────────────────
//  LINUX / macOS
// ─────────────────────────────────────────────────────────
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

namespace input {

static struct termios g_savedTermios;
static bool           g_rawMode = false;

static void enableRaw() {
    if (g_rawMode) return;
    struct termios t;
    tcgetattr(STDIN_FILENO, &g_savedTermios);
    t = g_savedTermios;
    t.c_lflag &= ~(ECHO | ICANON | ISIG);
    t.c_iflag &= ~(IXON | ICRNL);
    t.c_cc[VMIN]  = 0;
    t.c_cc[VTIME] = 0;   // fully non-blocking reads
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
    g_rawMode = true;
}

static void disableRaw() {
    if (!g_rawMode) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_savedTermios);
    g_rawMode = false;
}

// Block until data arrives (or timeoutMs elapses). Returns true if data ready.
static bool waitInput(int timeoutMs) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv = {timeoutMs / 1000, (timeoutMs % 1000) * 1000};
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
}

static int readByte() {
    unsigned char c;
    return (read(STDIN_FILENO, &c, 1) == 1) ? static_cast<int>(c) : -1;
}

void init()    {}
void cleanup() { disableRaw(); }

int readChar() {
    enableRaw();
    // Block until a byte arrives
    while (!waitInput(100)) {}
    int c = readByte();
    if (c < 0) return -1;
    // Eat escape sequences (arrow keys, F-keys)
    if (c == 27) {
        if (waitInput(50)) {
            int seq = readByte();
            if (seq == '[' && waitInput(50)) readByte();
        }
        return 27;
    }
    if (c == '\r') return '\n';
    return c;
}

LineResult readLine(const std::string& prompt) {
    enableRaw();
    if (!prompt.empty())
        std::cout << "\033[32m  > \033[0m" << prompt << ": ";
    std::cout.flush();

    std::string result;
    while (true) {
        int c = readChar();
        if (c == 0x11) { std::cout << "\n"; disableRaw(); return {"", true}; }
        if (c == '\n') { std::cout << "\n"; disableRaw(); return {result, false}; }
        if (c == 8 || c == 127) {
            if (!result.empty()) {
                result.pop_back();
                std::cout << "\b \b";
                std::cout.flush();
            }
            continue;
        }
        if (c >= 32 && c < 127) {
            result += static_cast<char>(c);
            std::cout << static_cast<char>(c);
            std::cout.flush();
        }
    }
}

bool hasKey() {
    enableRaw();
    return waitInput(0);
}

// Morse key timing on Linux.
// We use key-repeat events to estimate hold duration:
//   - Single char + 80 ms silence → short tap → dit
//   - Repeated chars until silence → dah (works for WPM ≤ ~8 where repeat kicks in)
// For higher WPM users are advised to use Windows or slow down slightly.
char waitMorseEvent(int keyChar, int ditMs, int timeoutMs) {
    enableRaw();

    // Drain stale input
    while (waitInput(0)) readByte();

    using clk = std::chrono::steady_clock;
    auto deadline = (timeoutMs >= 0)
        ? clk::now() + std::chrono::milliseconds(timeoutMs)
        : clk::time_point::max();

    // Wait for target key (or Ctrl+Q / timeout)
    while (true) {
        int pollMs = 50;
        if (timeoutMs >= 0) {
            auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - clk::now()).count();
            if (remaining <= 0) return '\0';
            pollMs = static_cast<int>(std::min((long long)50, remaining));
        }
        if (!waitInput(pollMs)) {
            if (timeoutMs >= 0 && clk::now() >= deadline) return '\0';
            continue;
        }
        int c = readByte();
        if (c < 0) continue;
        if (c == 0x11) return '\x11';   // Ctrl+Q
        if (c == keyChar) break;
        if (timeoutMs >= 0 && clk::now() >= deadline) return '\0';
    }

    // Measure hold time via key-repeat
    const int gapMs = 80;
    auto t0       = clk::now();
    auto lastSeen = t0;

    while (waitInput(gapMs)) {
        int c = readByte();
        if (c == keyChar)
            lastSeen = clk::now();
        else if (c == 0x11)
            return '\x11';
    }

    int ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            lastSeen - t0).count()) + gapMs / 2;

    return (ms < ditMs * 2) ? '.' : '-';
}

int nameToCode(const std::string& name) {
    std::string up = name;
    std::transform(up.begin(), up.end(), up.begin(), ::toupper);
    if (up == "SPACE") return ' ';
    if (up == "SHIFT") return 0;   // not reliably detectable in terminal
    if (up.size() == 1) return static_cast<int>(static_cast<unsigned char>(up[0]));
    return ' ';
}

std::string codeToName(int code) {
    if (code == ' ') return "SPACE";
    if (code >= 'A' && code <= 'Z') return std::string(1, static_cast<char>(code));
    if (code >= 'a' && code <= 'z') return std::string(1, static_cast<char>(std::toupper(code)));
    if (code >= '0' && code <= '9') return std::string(1, static_cast<char>(code));
    return "SPACE";
}

} // namespace input
#endif
