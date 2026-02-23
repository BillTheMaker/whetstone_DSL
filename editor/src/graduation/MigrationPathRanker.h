#pragma once
// Step 900: Migration path ranker — sorts candidates by feasibility descending.
#include <string>
#include <vector>
#include <algorithm>
#include "MigrationPathCandidate.h"

class MigrationPathRanker {
public:
    // Returns a new vector sorted by feasibility descending (highest first).
    static std::vector<MigrationPathCandidate> rank(
            std::vector<MigrationPathCandidate> candidates) {
        std::stable_sort(candidates.begin(), candidates.end(),
            [](const MigrationPathCandidate& a, const MigrationPathCandidate& b) {
                return a.feasibility > b.feasibility;
            });
        return candidates;
    }

    // Returns the top N candidates after ranking. If n >= size, returns all.
    static std::vector<MigrationPathCandidate> topN(
            std::vector<MigrationPathCandidate> candidates, std::size_t n) {
        auto ranked = rank(std::move(candidates));
        if (n < ranked.size()) ranked.resize(n);
        return ranked;
    }
};
