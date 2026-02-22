#pragma once
// Step 844: Auto-block rule integration with support tiers.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct AutoBlockRule {
    std::string ruleId;
    std::string tier;    // "stable", "beta", "experimental"
    std::string trigger; // "regression_critical", "cert_fail", "gate_fail"
    bool enabled = true;
};

struct BlockDecision {
    std::string pairId;
    bool blocked = false;
    std::string ruleId;
    std::string reason;
};

class AutoBlockRuleEngine {
public:
    void addRule(const AutoBlockRule& rule) { rules_.push_back(rule); }

    BlockDecision evaluate(const std::string& pairId,
                           const std::string& tier,
                           const std::string& trigger) const {
        BlockDecision d;
        d.pairId = pairId;
        for (const auto& r : rules_) {
            if (!r.enabled) continue;
            if (r.tier == tier && r.trigger == trigger) {
                d.blocked = true;
                d.ruleId = r.ruleId;
                d.reason = "rule_triggered:" + r.ruleId;
                return d;
            }
        }
        d.blocked = false;
        return d;
    }

    int ruleCount() const { return static_cast<int>(rules_.size()); }

    static nlohmann::json toJson(const BlockDecision& d) {
        return {{"pair_id", d.pairId}, {"blocked", d.blocked},
                {"rule_id", d.ruleId}, {"reason", d.reason}};
    }

private:
    std::vector<AutoBlockRule> rules_;
};
