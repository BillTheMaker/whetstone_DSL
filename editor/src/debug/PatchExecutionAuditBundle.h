#pragma once
// Step 1487: patch execution audit bundle.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "PatchExecutionRecord.h"

struct PatchExecutionAuditBundle {
    std::string sessionId;
    std::vector<PatchExecutionRecord> records;
    bool success = false;
};

class PatchExecutionAuditBundleModel {
public:
    static PatchExecutionAuditBundle build(const std::string& sessionId,
                                           const std::vector<PatchExecutionRecord>& records) {
        PatchExecutionAuditBundle b;
        b.sessionId = sessionId;
        b.records = records;
        b.success = !records.empty();
        return b;
    }

    static nlohmann::json toJson(const PatchExecutionAuditBundle& b) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& r : b.records) arr.push_back(PatchExecutionRecordModel::toJson(r));
        return {{"session_id", b.sessionId}, {"records", arr}, {"success", b.success}};
    }
};
