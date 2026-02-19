#pragma once
// Step 681: Batch quality audit over taskitems.

#include "SelfContainmentScorer.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct QualityAuditReport {
    int totalTaskitems = 0;
    int selfContainedCount = 0;
    int warningCount = 0;
    int failingCount = 0;
    double averageScore = 0.0;
    std::vector<ScoredTaskitem> results;
    std::vector<std::string> topIssues;
};

class TaskitemQualityAuditor {
public:
    static QualityAuditReport audit(const std::vector<TaskitemInput>& items,
                                    const std::string& workspaceRoot) {
        QualityAuditReport out;
        out.totalTaskitems = static_cast<int>(items.size());
        if (items.empty()) return out;

        int scoreSum = 0;
        std::map<std::string, int> issueCounts;
        out.results.reserve(items.size());

        for (const auto& item : items) {
            ScoredTaskitem scored = SelfContainmentScorer::score(item, workspaceRoot);
            scoreSum += scored.score;

            if (scored.score >= 80) ++out.selfContainedCount;
            else if (scored.score >= 50) ++out.warningCount;
            else ++out.failingCount;

            for (const auto& issue : scored.issues) {
                issueCounts[issue]++;
            }

            out.results.push_back(std::move(scored));
        }

        out.averageScore = static_cast<double>(scoreSum) / static_cast<double>(items.size());

        std::vector<std::pair<std::string, int>> issuePairs(issueCounts.begin(), issueCounts.end());
        std::sort(issuePairs.begin(), issuePairs.end(),
                  [](const auto& a, const auto& b) {
                      if (a.second != b.second) return a.second > b.second;
                      return a.first < b.first;
                  });
        for (const auto& p : issuePairs) out.topIssues.push_back(p.first);

        return out;
    }
};

