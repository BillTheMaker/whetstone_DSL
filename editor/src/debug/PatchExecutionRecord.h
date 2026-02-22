#pragma once
// Step 1479: patch execution record model.

#include <string>

#include <nlohmann/json.hpp>

struct PatchExecutionRecord {
    std::string executionId;
    std::string proposalId;
    std::string phase; // validated|dry_run|applied|rolled_back
    bool success = false;
    std::string detail;
};

class PatchExecutionRecordModel {
public:
    static PatchExecutionRecord make(const std::string& proposalId,
                                     const std::string& phase,
                                     bool success,
                                     const std::string& detail) {
        PatchExecutionRecord r;
        r.proposalId = proposalId;
        r.phase = phase;
        r.success = success;
        r.detail = detail;
        r.executionId = stableId(proposalId, phase, detail);
        return r;
    }

    static nlohmann::json toJson(const PatchExecutionRecord& r) {
        return {{"execution_id", r.executionId}, {"proposal_id", r.proposalId}, {"phase", r.phase},
                {"success", r.success}, {"detail", r.detail}};
    }

private:
    static std::string stableId(const std::string& p, const std::string& ph, const std::string& d) {
        uint64_t h = 5381;
        std::string key = p + "|" + ph + "|" + d;
        for (unsigned char c : key) h = ((h << 5) + h) + c;
        return "pe_" + std::to_string(h);
    }
};
