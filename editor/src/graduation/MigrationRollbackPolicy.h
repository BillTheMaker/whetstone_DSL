#pragma once
// Step 903: Migration rollback policy — defines what happens when a migration step fails.
#include <string>
#include <nlohmann/json.hpp>

struct RollbackPolicy {
    std::string pairId;
    std::string triggerCondition;   // human-readable description of when policy fires
    std::string action;             // "abort" | "checkpoint" | "retry"
    int maxRetries = 0;
};

class MigrationRollbackPolicy {
public:
    // Default policy: checkpoint on failure, allow up to 3 retries.
    static RollbackPolicy defaultPolicy(const std::string& pairId) {
        RollbackPolicy p;
        p.pairId           = pairId;
        p.triggerCondition = "any_step_failure";
        p.action           = "checkpoint";
        p.maxRetries       = 3;
        return p;
    }

    // Strict policy: abort immediately on first failure, no retries.
    static RollbackPolicy strict(const std::string& pairId) {
        RollbackPolicy p;
        p.pairId           = pairId;
        p.triggerCondition = "any_step_failure";
        p.action           = "abort";
        p.maxRetries       = 0;
        return p;
    }

    // Retry-only policy: retry up to N times before aborting.
    static RollbackPolicy retryPolicy(const std::string& pairId, int maxRetries) {
        RollbackPolicy p;
        p.pairId           = pairId;
        p.triggerCondition = "any_step_failure";
        p.action           = "retry";
        p.maxRetries       = maxRetries > 0 ? maxRetries : 1;
        return p;
    }

    static nlohmann::json toJson(const RollbackPolicy& p) {
        return {
            {"pair_id",           p.pairId},
            {"trigger_condition", p.triggerCondition},
            {"action",            p.action},
            {"max_retries",       p.maxRetries}
        };
    }
};
