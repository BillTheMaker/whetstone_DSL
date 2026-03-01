#pragma once
// Step 1875: Cross-artifact consistency engine.
// Checks that spec goal keywords appear in generated taskitem intents.
// Closes GR-015: previously CrossArtifactConsistencyChecker was a data stub;
// this provides the actual evaluation logic.

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

struct ConsistencyResult {
    int inconsistencyCount = 0;
    bool consistent = false;
    std::vector<std::string> driftedGoals;
};

class CrossArtifactConsistencyEngine {
public:
    // Check that each spec goal has at least one meaningful keyword present
    // in at least one taskitem intent. Goals with no coverage are drift.
    static ConsistencyResult check(const std::vector<std::string>& specGoals,
                                   const std::vector<std::string>& taskitemIntents) {
        ConsistencyResult result;
        for (const auto& goal : specGoals) {
            if (!anyIntentCoversGoal(goal, taskitemIntents)) {
                ++result.inconsistencyCount;
                result.driftedGoals.push_back(goal);
            }
        }
        result.consistent = result.inconsistencyCount == 0;
        return result;
    }

private:
    static std::vector<std::string> tokenize(const std::string& text) {
        std::vector<std::string> tokens;
        std::string tok;
        for (unsigned char c : text) {
            if (std::isalnum(c)) {
                tok += static_cast<char>(std::tolower(c));
            } else if (!tok.empty()) {
                tokens.push_back(tok);
                tok.clear();
            }
        }
        if (!tok.empty()) tokens.push_back(tok);
        return tokens;
    }

    static bool isStopWord(const std::string& tok) {
        static const std::vector<std::string> stops = {
            "the", "and", "must", "should", "will", "that", "with", "for", "are", "has"
        };
        return std::find(stops.begin(), stops.end(), tok) != stops.end();
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        for (auto& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }

    static bool anyIntentCoversGoal(const std::string& goal,
                                    const std::vector<std::string>& intents) {
        auto goalTokens = tokenize(goal);
        goalTokens.erase(
            std::remove_if(goalTokens.begin(), goalTokens.end(), isStopWord),
            goalTokens.end());

        // Only consider tokens of meaningful length (avoid short noise words)
        std::vector<std::string> meaningful;
        for (const auto& tok : goalTokens) {
            if (tok.size() >= 4) meaningful.push_back(tok);
        }
        if (meaningful.empty()) return true; // no meaningful tokens → not flagged

        // A goal is covered if at least one meaningful token appears in any intent
        for (const auto& tok : meaningful) {
            for (const auto& intent : intents) {
                if (toLower(intent).find(tok) != std::string::npos) return true;
            }
        }
        return false;
    }
};
