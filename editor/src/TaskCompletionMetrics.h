#pragma once
// Step 685: Task completion outcome metrics.

#include <algorithm>
#include <string>
#include <vector>

struct TaskOutcome {
    std::string taskId;
    int linesChanged = 0;
    int linesAdded = 0;
    int linesRemoved = 0;
    bool testsProvided = false;
    bool testsPassed = false;
    int qualityRating = 0;
    std::string notes;
};

class TaskCompletionMetrics {
public:
    static TaskOutcome diff(const std::string& taskId,
                            const std::string& before,
                            const std::string& after,
                            bool testsPassed,
                            int qualityRating,
                            const std::string& notes = "") {
        TaskOutcome out;
        out.taskId = taskId;
        out.testsPassed = testsPassed;
        out.testsProvided = testsPassed;
        out.qualityRating = std::clamp(qualityRating, 0, 100);
        out.notes = notes;

        const auto beforeLines = splitLines(before);
        const auto afterLines = splitLines(after);

        size_t prefix = 0;
        while (prefix < beforeLines.size() &&
               prefix < afterLines.size() &&
               beforeLines[prefix] == afterLines[prefix]) {
            ++prefix;
        }

        size_t suffix = 0;
        while (suffix + prefix < beforeLines.size() &&
               suffix + prefix < afterLines.size() &&
               beforeLines[beforeLines.size() - 1 - suffix] ==
               afterLines[afterLines.size() - 1 - suffix]) {
            ++suffix;
        }

        const int removed = static_cast<int>(beforeLines.size() - prefix - suffix);
        const int added = static_cast<int>(afterLines.size() - prefix - suffix);
        out.linesRemoved = std::max(0, removed);
        out.linesAdded = std::max(0, added);
        out.linesChanged = out.linesAdded + out.linesRemoved;
        return out;
    }

    static int countLines(const std::string& text) {
        if (text.empty()) return 0;
        int lines = 1;
        for (char c : text) if (c == '\n') ++lines;
        if (!text.empty() && text.back() == '\n') --lines;
        return lines;
    }

private:
    static std::vector<std::string> splitLines(const std::string& text) {
        std::vector<std::string> out;
        std::string current;
        for (char c : text) {
            if (c == '\n') {
                out.push_back(current);
                current.clear();
            } else {
                current.push_back(c);
            }
        }
        if (!text.empty() && text.back() != '\n') {
            out.push_back(current);
        }
        return out;
    }
};

