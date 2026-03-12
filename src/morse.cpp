#include "morse.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace morse {

static const std::unordered_map<char, std::string> encodeTable = {
    {'A', ".-"},    {'B', "-..."},  {'C', "-.-."},  {'D', "-.."},
    {'E', "."},     {'F', "..-."},  {'G', "--."},   {'H', "...."},
    {'I', ".."},    {'J', ".---"},  {'K', "-.-"},   {'L', ".-.."},
    {'M', "--"},    {'N', "-."},    {'O', "---"},   {'P', ".--."},
    {'Q', "--.-"},  {'R', ".-."},   {'S', "..."},   {'T', "-"},
    {'U', "..-"},   {'V', "...-"},  {'W', ".--"},   {'X', "-..-"},
    {'Y', "-.--"},  {'Z', "--.."},
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"},
    {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."},
    {'8', "---.."}, {'9', "----."},
    {'.', ".-.-.-"}, {',', "--..--"}, {'?', "..--.."}, {'\'', ".----."},
    {'!', "-.-.--"}, {'/', "-..-."}, {'(', "-.--."}, {')', "-.--.-"},
    {'&', ".-..."}, {':', "---..."}, {';', "-.-.-."}, {'=', "-...-"},
    {'+', ".-.-."}, {'-', "-....-"}, {'_', "..--.-"}, {'"', ".-..-."},
    {'$', "...-..-"}, {'@', ".--.-."},
};

static std::unordered_map<std::string, char> buildDecodeTable() {
    std::unordered_map<std::string, char> table;
    for (auto& [ch, code] : encodeTable) {
        table[code] = ch;
    }
    return table;
}

static const std::unordered_map<std::string, char> decodeTable = buildDecodeTable();

std::string encode(char c) {
    char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    auto it = encodeTable.find(upper);
    if (it != encodeTable.end()) return it->second;
    return "";
}

char decode(const std::string& code) {
    auto it = decodeTable.find(code);
    if (it != decodeTable.end()) return it->second;
    return '\0';
}

std::string encodeText(const std::string& text) {
    std::string result;
    bool firstChar = true;
    bool prevWasSpace = false;

    for (char c : text) {
        if (c == ' ') {
            if (!firstChar && !prevWasSpace) {
                result += " / ";
                prevWasSpace = true;
            }
            continue;
        }
        std::string code = encode(c);
        if (code.empty()) continue;

        if (!firstChar && !prevWasSpace) {
            result += " ";
        }
        result += code;
        firstChar = false;
        prevWasSpace = false;
    }
    return result;
}

std::string decodeText(const std::string& morseText) {
    std::string result;
    std::istringstream stream(morseText);
    std::string token;

    while (stream >> token) {
        if (token == "/") {
            result += ' ';
        } else {
            char ch = decode(token);
            if (ch != '\0') {
                result += ch;
            }
        }
    }
    return result;
}

std::vector<char> getAllCharacters() {
    std::vector<char> chars;
    chars.reserve(encodeTable.size());
    for (auto& [ch, code] : encodeTable) {
        chars.push_back(ch);
    }
    std::sort(chars.begin(), chars.end());
    return chars;
}

const std::unordered_map<char, std::string>& getEncodeTable() {
    return encodeTable;
}

bool isSupported(char c) {
    char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return encodeTable.count(upper) > 0;
}

} // namespace morse
