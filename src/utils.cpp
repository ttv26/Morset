#include "utils.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace utils {

static std::mt19937& getRng() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

char randomChar(const std::vector<char>& chars) {
    if (chars.empty()) return 'A';
    std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);
    return chars[dist(getRng())];
}

static const std::vector<std::string> words = {
    "THE", "BE", "TO", "OF", "AND", "A", "IN", "THAT", "HAVE", "I",
    "IT", "FOR", "NOT", "ON", "WITH", "HE", "AS", "YOU", "DO", "AT",
    "THIS", "BUT", "HIS", "BY", "FROM", "THEY", "WE", "SAY", "HER", "SHE",
    "OR", "AN", "WILL", "MY", "ONE", "ALL", "WOULD", "THERE", "THEIR", "WHAT",
    "SO", "UP", "OUT", "IF", "ABOUT", "WHO", "GET", "WHICH", "GO", "ME",
    "WHEN", "MAKE", "CAN", "LIKE", "TIME", "NO", "JUST", "HIM", "KNOW", "TAKE",
    "COME", "COULD", "THAN", "LOOK", "DAY", "HAD", "ONLY", "INTO", "YEAR", "SOME",
    "HELP", "CODE", "CALL", "RADIO", "SEND", "STOP", "MORSE", "SIGNAL", "OVER", "COPY",
    "ROGER", "ALPHA", "BRAVO", "DELTA", "ECHO", "GOLF", "HOTEL", "INDIA", "KILO", "LIMA",
    "MIKE", "OSCAR", "PAPA", "ROMEO", "TANGO", "VICTOR", "ZULU", "SOS", "CQ", "QSL"
};

std::string randomWord() {
    std::uniform_int_distribution<size_t> dist(0, words.size() - 1);
    return words[dist(getRng())];
}

static const std::vector<std::string> sentences = {
    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG",
    "SEND HELP NOW",
    "CQ CQ CQ",
    "MORSE CODE IS FUN",
    "PRACTICE MAKES PERFECT",
    "SOS SOS SOS",
    "ROGER THAT COPY",
    "HELLO WORLD",
    "GOOD MORNING",
    "HOW ARE YOU",
    "THE WEATHER IS CLEAR",
    "SIGNAL IS STRONG",
    "PLEASE REPEAT",
    "ALL IS WELL",
    "OVER AND OUT",
    "STAND BY FOR MESSAGE",
    "ALPHA BRAVO CHARLIE",
    "ZERO NINE EIGHT SEVEN",
    "HAPPY TO HELP",
    "KEEP UP THE GOOD WORK"
};

std::string randomSentence() {
    std::uniform_int_distribution<size_t> dist(0, sentences.size() - 1);
    return sentences[dist(getRng())];
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return result;
}

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string getDataDir() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, path) == S_OK) {
        return std::string(path) + "\\MorseTrainer";
    }
    return ".\\morse_trainer_data";
#else
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home) + "/.morse_trainer";
    }
    return "./morse_trainer_data";
#endif
}

void ensureDataDir() {
    std::string dir = getDataDir();
    std::filesystem::create_directories(dir);
}

} // namespace utils
