#pragma once
// Step 849: Transpilation failure taxonomy schema.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Canonical taxonomy codes
// T001 = semantic_mismatch, T002 = type_gap, T003 = control_flow
// T004 = memory_model, T005 = concurrency, T006 = stdlib_gap
// T007 = runtime_behavior, T008 = syntax_unsupported

struct TaxonomyCode {
    std::string code;
    std::string category;
    std::string description;
    std::string severity;
};

struct TaggedFailure {
    std::string failureId;
    std::string pairId;
    std::string primaryCode;
    std::vector<std::string> secondaryCodes;
    std::string message;
};

class FailureTaxonomy {
public:
    static std::vector<TaxonomyCode> allCodes() {
        return {
            {"T001", "semantic",  "semantic_mismatch",   "high"},
            {"T002", "type",      "type_gap",             "medium"},
            {"T003", "control",   "control_flow_diff",    "medium"},
            {"T004", "memory",    "memory_model_gap",     "high"},
            {"T005", "concurr",   "concurrency_gap",      "high"},
            {"T006", "stdlib",    "stdlib_gap",           "low"},
            {"T007", "runtime",   "runtime_behavior_diff","high"},
            {"T008", "syntax",    "syntax_unsupported",   "medium"},
        };
    }

    static bool isValid(const std::string& code) {
        for (const auto& c : allCodes()) if (c.code == code) return true;
        return false;
    }

    static TaggedFailure tag(const std::string& failureId, const std::string& pairId,
                              const std::string& primaryCode, const std::string& msg) {
        TaggedFailure f;
        f.failureId = failureId;
        f.pairId = pairId;
        f.primaryCode = primaryCode;
        f.message = msg;
        return f;
    }

    static nlohmann::json toJson(const TaggedFailure& f) {
        nlohmann::json sec = nlohmann::json::array();
        for (const auto& s : f.secondaryCodes) sec.push_back(s);
        return {{"failure_id", f.failureId}, {"pair_id", f.pairId},
                {"primary_code", f.primaryCode}, {"message", f.message}, {"secondary", sec}};
    }
};
