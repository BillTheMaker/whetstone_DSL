#pragma once
// Step 847: Certification dashboard data model.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct DashboardEntry {
    std::string pairId;
    std::string lastCycle;
    std::string status;    // "certified", "failed", "pending"
    int passRate = 0;
    std::string tier;
};

struct CertificationDashboardState {
    std::vector<DashboardEntry> entries;
    int certified = 0;
    int failed = 0;
    int pending = 0;
    std::string generatedAt;
};

class CertificationDashboard {
public:
    static CertificationDashboardState build(const std::vector<DashboardEntry>& entries) {
        CertificationDashboardState s;
        s.entries = entries;
        for (const auto& e : entries) {
            if (e.status == "certified")   ++s.certified;
            else if (e.status == "failed") ++s.failed;
            else                           ++s.pending;
        }
        return s;
    }

    static nlohmann::json toJson(const CertificationDashboardState& s) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& e : s.entries)
            arr.push_back({{"pair_id", e.pairId}, {"status", e.status},
                           {"pass_rate", e.passRate}, {"tier", e.tier}});
        return {{"certified", s.certified}, {"failed", s.failed},
                {"pending", s.pending}, {"entries", arr}};
    }
};
