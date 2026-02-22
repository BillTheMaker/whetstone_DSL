#pragma once
// Step 827: Governance and audit report bundle.

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct AuditEntry {
    std::string entryId;
    std::string category;  // "decision", "waiver", "policy"
    std::string actorId;
    std::string action;
    std::string timestamp;
    std::string details;
};

struct GovernanceAuditReport {
    std::string reportId;
    std::vector<AuditEntry> entries;
    int totalDecisions = 0;
    int totalWaivers = 0;
    int totalPolicyChanges = 0;
    bool compliant = true;
};

class GovernanceAuditReportModel {
public:
    static GovernanceAuditReport build(const std::string& reportId,
                                       const std::vector<AuditEntry>& entries) {
        GovernanceAuditReport r;
        r.reportId = reportId;
        r.entries = entries;
        for (const auto& e : entries) {
            if (e.category == "decision")      ++r.totalDecisions;
            else if (e.category == "waiver")   ++r.totalWaivers;
            else if (e.category == "policy")   ++r.totalPolicyChanges;
        }
        r.compliant = true;
        return r;
    }

    static bool validate(const GovernanceAuditReport& r, std::string* error) {
        if (!error) return false;
        error->clear();
        if (r.reportId.empty()) { *error = "report_id_missing"; return false; }
        return true;
    }

    static nlohmann::json toJson(const GovernanceAuditReport& r) {
        nlohmann::json entries = nlohmann::json::array();
        for (const auto& e : r.entries) {
            entries.push_back({{"entry_id", e.entryId}, {"category", e.category},
                               {"actor_id", e.actorId}, {"action", e.action},
                               {"timestamp", e.timestamp}, {"details", e.details}});
        }
        return {{"report_id", r.reportId}, {"entries", entries},
                {"total_decisions", r.totalDecisions}, {"total_waivers", r.totalWaivers},
                {"total_policy_changes", r.totalPolicyChanges}, {"compliant", r.compliant}};
    }
};
