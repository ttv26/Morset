#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <ctime>

namespace stats {

struct CharStat {
    int attempts = 0;
    int correct = 0;
    double errorRate() const { return attempts > 0 ? 1.0 - (static_cast<double>(correct) / attempts) : 0.0; }
    double accuracy() const { return attempts > 0 ? (static_cast<double>(correct) / attempts) * 100.0 : 0.0; }
};

struct SessionRecord {
    time_t timestamp = 0;
    int totalAttempts = 0;
    int totalCorrect = 0;
    double accuracy() const { return totalAttempts > 0 ? (static_cast<double>(totalCorrect) / totalAttempts) * 100.0 : 0.0; }
};

class StatsManager {
public:
    StatsManager();

    // Record a single character attempt
    void recordAttempt(char c, bool correct);

    // Get stats for a specific character
    CharStat getCharStat(char c) const;

    // Get all character stats
    const std::unordered_map<char, CharStat>& getAllCharStats() const;

    // End current session and save to history
    void endSession();

    // Get session history
    const std::vector<SessionRecord>& getSessionHistory() const;

    // Display per-letter performance table
    void showLetterStats() const;

    // Display ASCII progress graph
    void showProgressGraph() const;

    // Display combined stats dashboard
    void showDashboard() const;

    // Save stats to file
    void save() const;

    // Load stats from file
    void load();

    // Reset all stats
    void reset();

    // Get characters sorted by error rate (worst first) for targeted practice
    std::vector<std::pair<char, CharStat>> getWeakestChars(int count = 10) const;

private:
    std::unordered_map<char, CharStat> charStats_;
    std::vector<SessionRecord> sessionHistory_;

    // Current session accumulators
    int sessionAttempts_ = 0;
    int sessionCorrect_ = 0;

    std::string getStatsFilePath() const;
};

} // namespace stats
