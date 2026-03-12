#include "stats.h"
#include "ui.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <cmath>

namespace stats {

StatsManager::StatsManager() {
    load();
}

void StatsManager::recordAttempt(char c, bool correct) {
    char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    charStats_[upper].attempts++;
    if (correct) charStats_[upper].correct++;
    sessionAttempts_++;
    if (correct) sessionCorrect_++;
}

CharStat StatsManager::getCharStat(char c) const {
    char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    auto it = charStats_.find(upper);
    if (it != charStats_.end()) return it->second;
    return CharStat{};
}

const std::unordered_map<char, CharStat>& StatsManager::getAllCharStats() const {
    return charStats_;
}

void StatsManager::endSession() {
    if (sessionAttempts_ > 0) {
        SessionRecord record;
        record.timestamp = std::time(nullptr);
        record.totalAttempts = sessionAttempts_;
        record.totalCorrect = sessionCorrect_;
        sessionHistory_.push_back(record);
    }
    sessionAttempts_ = 0;
    sessionCorrect_ = 0;
    save();
}

const std::vector<SessionRecord>& StatsManager::getSessionHistory() const {
    return sessionHistory_;
}

void StatsManager::showLetterStats() const {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Per-Letter Performance ===")) << "\n\n";

    // Collect and sort by character
    std::vector<std::pair<char, CharStat>> sorted(charStats_.begin(), charStats_.end());
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) { return a.first < b.first; });

    if (sorted.empty()) {
        std::cout << ui::colorDim("  No data yet. Complete some training sessions first!") << "\n\n";
        ui::pressEnterToContinue();
        return;
    }

    // Header
    std::cout << "  " << ui::colorCyan(std::string(58, '-')) << "\n";
    std::cout << "  " << std::left << std::setw(6) << "Char"
              << std::setw(10) << "Attempts"
              << std::setw(10) << "Correct"
              << std::setw(12) << "Accuracy"
              << "Bar" << "\n";
    std::cout << "  " << ui::colorCyan(std::string(58, '-')) << "\n";

    for (auto& [ch, stat] : sorted) {
        double acc = stat.accuracy();
        std::string accStr = std::to_string(static_cast<int>(acc)) + "%";

        // Color based on accuracy
        std::string accColored;
        if (acc >= 80.0) accColored = ui::colorGreen(accStr);
        else if (acc >= 50.0) accColored = ui::colorYellow(accStr);
        else accColored = ui::colorRed(accStr);

        // Mini bar (20 chars wide)
        int barLen = static_cast<int>(acc / 5.0);
        std::string bar;
        for (int i = 0; i < 20; ++i) {
            if (i < barLen) bar += "█";
            else bar += "░";
        }
        std::string barColored;
        if (acc >= 80.0) barColored = ui::colorGreen(bar);
        else if (acc >= 50.0) barColored = ui::colorYellow(bar);
        else barColored = ui::colorRed(bar);

        std::cout << "  " << ui::colorBold(std::string(1, ch)) << "     "
                  << std::left << std::setw(10) << stat.attempts
                  << std::setw(10) << stat.correct
                  << std::setw(12) << accColored
                  << barColored << "\n";
    }

    std::cout << "  " << ui::colorCyan(std::string(58, '-')) << "\n\n";

    // Show weakest characters
    auto weakest = getWeakestChars(5);
    if (!weakest.empty()) {
        std::cout << ui::colorRed("  Weakest characters (practice these!): ");
        for (auto& [ch, stat] : weakest) {
            std::cout << ch << "(" << static_cast<int>(stat.accuracy()) << "%) ";
        }
        std::cout << "\n\n";
    }

    ui::pressEnterToContinue();
}

void StatsManager::showProgressGraph() const {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Progress Graph (Session Accuracy Over Time) ===")) << "\n\n";

    if (sessionHistory_.empty()) {
        std::cout << ui::colorDim("  No session history yet. Complete some training sessions first!") << "\n\n";
        ui::pressEnterToContinue();
        return;
    }

    const int graphHeight = 15;
    const int graphWidth = std::min(static_cast<int>(sessionHistory_.size()), 40);

    // Get the last N sessions to display
    int startIdx = std::max(0, static_cast<int>(sessionHistory_.size()) - graphWidth);
    std::vector<double> accuracies;
    for (int i = startIdx; i < static_cast<int>(sessionHistory_.size()); ++i) {
        accuracies.push_back(sessionHistory_[i].accuracy());
    }

    // Draw graph (top to bottom)
    for (int row = graphHeight; row >= 0; --row) {
        double threshold = (static_cast<double>(row) / graphHeight) * 100.0;
        // Y-axis label
        if (row % 3 == 0) {
            std::cout << std::right << std::setw(5) << static_cast<int>(threshold) << "% │";
        } else {
            std::cout << "      │";
        }

        for (size_t col = 0; col < accuracies.size(); ++col) {
            if (accuracies[col] >= threshold) {
                double acc = accuracies[col];
                if (acc >= 80.0) std::cout << ui::colorGreen("█");
                else if (acc >= 50.0) std::cout << ui::colorYellow("█");
                else std::cout << ui::colorRed("█");
            } else {
                std::cout << " ";
            }
        }
        std::cout << "\n";
    }

    // X-axis
    std::cout << "      └";
    for (size_t i = 0; i < accuracies.size(); ++i) std::cout << "─";
    std::cout << "─>\n";
    std::cout << "       " << ui::colorDim("Sessions (oldest → newest)") << "\n\n";

    // Summary stats
    double totalAcc = 0;
    for (double a : accuracies) totalAcc += a;
    double avgAcc = totalAcc / accuracies.size();
    double latestAcc = accuracies.back();
    double bestAcc = *std::max_element(accuracies.begin(), accuracies.end());

    std::cout << "  " << ui::colorCyan("Sessions shown: ") << accuracies.size() << "\n";
    std::cout << "  " << ui::colorCyan("Average accuracy: ") << std::fixed << std::setprecision(1) << avgAcc << "%\n";
    std::cout << "  " << ui::colorCyan("Latest accuracy:  ") << std::fixed << std::setprecision(1) << latestAcc << "%\n";
    std::cout << "  " << ui::colorCyan("Best accuracy:    ") << std::fixed << std::setprecision(1) << bestAcc << "%\n";

    // Trend indicator
    if (accuracies.size() >= 3) {
        double recent = (accuracies[accuracies.size()-1] + accuracies[accuracies.size()-2]) / 2.0;
        double older = (accuracies[0] + accuracies[1]) / 2.0;
        if (recent > older + 5) {
            std::cout << "  " << ui::colorGreen("  ▲ Trending UP! Great progress!") << "\n";
        } else if (recent < older - 5) {
            std::cout << "  " << ui::colorRed("  ▼ Trending down. Keep practicing!") << "\n";
        } else {
            std::cout << "  " << ui::colorYellow("  ► Steady performance.") << "\n";
        }
    }

    std::cout << "\n";
    ui::pressEnterToContinue();
}

void StatsManager::showDashboard() const {
    ui::clearScreen();
    std::cout << "\n" << ui::colorBold(ui::colorYellow("  === Statistics Dashboard ===")) << "\n\n";

    // Overall stats
    int totalAttempts = 0, totalCorrect = 0;
    for (auto& [ch, stat] : charStats_) {
        totalAttempts += stat.attempts;
        totalCorrect += stat.correct;
    }

    double overallAcc = totalAttempts > 0 ? (static_cast<double>(totalCorrect) / totalAttempts) * 100.0 : 0.0;

    std::cout << ui::colorCyan("  Overall Statistics:") << "\n";
    std::cout << "  Total attempts:    " << totalAttempts << "\n";
    std::cout << "  Total correct:     " << totalCorrect << "\n";
    std::cout << "  Overall accuracy:  " << std::fixed << std::setprecision(1) << overallAcc << "%\n";
    std::cout << "  Characters learned:" << charStats_.size() << "\n";
    std::cout << "  Sessions completed:" << sessionHistory_.size() << "\n";
    ui::printSeparator();

    // Accuracy meter
    std::cout << "\n  " << ui::colorCyan("Accuracy: ");
    ui::printProgressBar(overallAcc / 100.0, 30);

    std::cout << "\n";

    // Sub-menu
    std::vector<std::string> opts = {
        "View per-letter stats",
        "View progress graph",
        "Reset all stats",
        "Back to main menu"
    };

    int choice = ui::showMenu("What would you like to see?", opts);

    switch (choice) {
        case 1: showLetterStats(); break;
        case 2: showProgressGraph(); break;
        case 3: {
            std::string confirm = ui::promptString("Type 'RESET' to confirm");
            if (confirm == "RESET") {
                const_cast<StatsManager*>(this)->reset();
                std::cout << ui::colorGreen("  Stats reset successfully!") << "\n";
                ui::pressEnterToContinue();
            }
            break;
        }
        case 4: break;
    }
}

void StatsManager::save() const {
    utils::ensureDataDir();
    std::string path = getStatsFilePath();
    std::ofstream file(path);
    if (!file.is_open()) return;

    // Write character stats
    file << "[CHARS]\n";
    for (auto& [ch, stat] : charStats_) {
        file << ch << " " << stat.attempts << " " << stat.correct << "\n";
    }

    // Write session history
    file << "[SESSIONS]\n";
    for (auto& session : sessionHistory_) {
        file << session.timestamp << " " << session.totalAttempts << " " << session.totalCorrect << "\n";
    }
}

void StatsManager::load() {
    std::string path = getStatsFilePath();
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    std::string section;

    while (std::getline(file, line)) {
        if (line == "[CHARS]") { section = "CHARS"; continue; }
        if (line == "[SESSIONS]") { section = "SESSIONS"; continue; }

        std::istringstream iss(line);
        if (section == "CHARS") {
            char ch;
            int attempts, correct;
            if (iss >> ch >> attempts >> correct) {
                charStats_[ch] = {attempts, correct};
            }
        } else if (section == "SESSIONS") {
            time_t ts;
            int attempts, correct;
            if (iss >> ts >> attempts >> correct) {
                sessionHistory_.push_back({ts, attempts, correct});
            }
        }
    }
}

void StatsManager::reset() {
    charStats_.clear();
    sessionHistory_.clear();
    sessionAttempts_ = 0;
    sessionCorrect_ = 0;
    save();
}

std::vector<std::pair<char, CharStat>> StatsManager::getWeakestChars(int count) const {
    std::vector<std::pair<char, CharStat>> sorted(charStats_.begin(), charStats_.end());
    // Only include chars with at least 3 attempts
    sorted.erase(std::remove_if(sorted.begin(), sorted.end(),
        [](auto& p) { return p.second.attempts < 3; }), sorted.end());

    // Sort by error rate (highest first)
    std::sort(sorted.begin(), sorted.end(),
        [](auto& a, auto& b) { return a.second.errorRate() > b.second.errorRate(); });

    if (static_cast<int>(sorted.size()) > count) {
        sorted.resize(count);
    }
    return sorted;
}

std::string StatsManager::getStatsFilePath() const {
    return utils::getDataDir() + 
#ifdef _WIN32
        "\\stats.dat";
#else
        "/stats.dat";
#endif
}

} // namespace stats
