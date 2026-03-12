#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace morse {

// Returns the Morse code string for a given character (e.g., 'A' -> ".-")
std::string encode(char c);

// Returns the character for a given Morse code string (e.g., ".-" -> 'A')
char decode(const std::string& code);

// Encode a full string to Morse (letters separated by " ", words by " / ")
std::string encodeText(const std::string& text);

// Decode Morse string back to text
std::string decodeText(const std::string& morseText);

// Get all supported characters
std::vector<char> getAllCharacters();

// Get the full Morse code table
const std::unordered_map<char, std::string>& getEncodeTable();

// Check if a character is supported
bool isSupported(char c);

} // namespace morse
