#pragma once
// Step 1512: patch risk labeler model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct PatchRiskLabel {
    std::string level = "low";
    std::vector<std::string> rationale;
};

class PatchRiskLabeler {
public:
    static PatchRiskLabel label(int filesTouched,
                                int lineChanges,
                                bool corePathTouched,
                                bool addsUnsafePattern) {
        PatchRiskLabel r;
        if (filesTouched >= 5 || lineChanges >= 80 || corePathTouched || addsUnsafePattern) {
            r.level = "high";
        } else if (filesTouched >= 3 || lineChanges >= 30) {
            r.level = "medium";
        }
        if (filesTouched >= 3) r.rationale.push_back("multi_file_change");
        if (lineChanges >= 30) r.rationale.push_back("large_diff");
        if (corePathTouched) r.rationale.push_back("core_path_touch");
        if (addsUnsafePattern) r.rationale.push_back("unsafe_pattern");
        if (r.rationale.empty()) r.rationale.push_back("contained_change");
        return r;
    }

    static nlohmann::json toJson(const PatchRiskLabel& r) {
        return {{"level", r.level}, {"rationale", r.rationale}};
    }
};
