#pragma once
// Step 861: Deterministic fallback contract when hints unavailable.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct FallbackDecision {
    std::string pairId;
    std::string rule;         // deterministic rule applied
    std::string action;
    bool usedFallback = true;
    std::string reason;
};

class DeterministicFallback {
public:
    static FallbackDecision apply(const std::string& pairId,
                                   const std::string& context,
                                   bool hintsAvailable) {
        FallbackDecision d;
        d.pairId = pairId;
        d.usedFallback = !hintsAvailable;
        if (!hintsAvailable) {
            d.rule = "safe_first_default";
            d.action = "apply_conservative_transform";
            d.reason = "no_hint_model_available";
        } else {
            d.rule = "hint_guided";
            d.action = "apply_hint_suggestion";
            d.reason = "hints_available";
        }
        (void)context;
        return d;
    }

    static bool validate(const FallbackDecision& d, std::string* error = nullptr) {
        if (d.pairId.empty()) { if (error) *error = "pair_id_missing"; return false; }
        if (d.rule.empty()) { if (error) *error = "rule_missing"; return false; }
        return true;
    }

    static nlohmann::json toJson(const FallbackDecision& d) {
        return {{"pair_id", d.pairId}, {"rule", d.rule}, {"action", d.action},
                {"used_fallback", d.usedFallback}, {"reason", d.reason}};
    }
};
