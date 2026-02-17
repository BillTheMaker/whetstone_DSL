#pragma once

#include "CodeAnnotationBadges.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct HeatmapCell {
    int line = 0;
    int score = 0;
    std::string colorHex;
};

class CodeAnnotationHeatmap {
public:
    static std::vector<HeatmapCell> build(const std::vector<AnnotationBadgeInput>& annotations,
                                          int maxLine,
                                          bool enabled = true) {
        if (!enabled || maxLine <= 0) return {};
        std::map<int, int> scores;
        for (const auto& a : annotations) {
            if (a.line <= 0 || a.line > maxLine) continue;
            scores[a.line] += weightForType(a.type);
        }
        std::vector<HeatmapCell> out;
        for (int line = 1; line <= maxLine; ++line) {
            int score = scores.count(line) ? scores[line] : 0;
            out.push_back({line, score, colorForScore(score)});
        }
        return out;
    }

    static int weightForType(const std::string& type) {
        if (type.find("Complexity") != std::string::npos) return 3;
        if (type.find("Risk") != std::string::npos) return 4;
        if (type.find("Review") != std::string::npos) return 2;
        if (type.find("Contract") != std::string::npos) return 1;
        if (type.find("Intent") != std::string::npos) return 1;
        return 1;
    }

    static std::string colorForScore(int score) {
        if (score <= 0) return "#1F6F8B";     // cool blue (no annotations)
        if (score <= 2) return "#2ECC71";     // cool green (well-annotated simple)
        if (score <= 5) return "#F39C12";     // warm orange (moderate complexity)
        return "#E74C3C";                     // hot red (high complexity/risk)
    }
};
