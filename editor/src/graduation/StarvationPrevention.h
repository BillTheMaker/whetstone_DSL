#pragma once
// Step 882: Starvation prevention mechanism.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct StarvationRiskEntry {
    std::string pairId;
    int waitCycles = 0;
    bool atRisk = false;
    bool promoted = false;
};

class StarvationPrevention {
public:
    static StarvationRiskEntry assess(const std::string& pairId, int waitCycles,
                                       int starvationThreshold = 5) {
        StarvationRiskEntry e;
        e.pairId = pairId;
        e.waitCycles = waitCycles;
        e.atRisk = waitCycles >= starvationThreshold;
        e.promoted = e.atRisk;
        return e;
    }

    static int countAtRisk(const std::vector<StarvationRiskEntry>& entries) {
        int n = 0; for (const auto& e : entries) if (e.atRisk) ++n; return n;
    }

    static nlohmann::json toJson(const StarvationRiskEntry& e) {
        return {{"pair_id", e.pairId}, {"wait_cycles", e.waitCycles},
                {"at_risk", e.atRisk}, {"promoted", e.promoted}};
    }
};
