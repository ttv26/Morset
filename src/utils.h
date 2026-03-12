#pragma once
#include <string>
#include <vector>
#include <random>

namespace utils {

// Get a random character from the given set
char randomChar(const std::vector<char>& chars);

// Get a random word from a built-in word list
std::string randomWord();

// Get a random sentence from a built-in list
std::string randomSentence();

// Convert string to uppercase
std::string toUpper(const std::string& str);

// Trim whitespace from both ends
std::string trim(const std::string& str);

// Get the data directory path for saving stats
std::string getDataDir();

// Ensure data directory exists
void ensureDataDir();

} // namespace utils
