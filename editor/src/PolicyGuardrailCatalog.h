#pragma once
// Step 594: Policy Guardrail Catalog

#include <map>
#include <string>
#include <vector>

enum class GuardrailDecision {
    Allow,
    RequireReview,
    Deny
};

struct GuardrailRule {
    std::string ruleId;
    std::string operationPrefix;
    GuardrailDecision decision = GuardrailDecision::RequireReview;
    std::string reason;
};

struct GuardrailDecisionResult {
    GuardrailDecision decision = GuardrailDecision::RequireReview;
    std::string matchedRuleId;
    std::string reason;
};

class PolicyGuardrailCatalog {
public:
    bool addRule(const GuardrailRule& rule, std::string* error) {
        if (!error) return false;
        error->clear();
        if (rule.ruleId.empty()) return fail(error, "rule_id_missing");
        if (rule.operationPrefix.empty()) return fail(error, "operation_prefix_missing");
        if (rule.reason.empty()) return fail(error, "rule_reason_missing");
        if (rules_.count(rule.ruleId) != 0) return fail(error, "rule_duplicate");
        rules_[rule.ruleId] = rule;
        order_.push_back(rule.ruleId);
        return true;
    }

    GuardrailDecisionResult evaluate(const std::string& operation) const {
        GuardrailDecisionResult result;
        result.decision = GuardrailDecision::RequireReview;
        result.reason = "default_review_required";
        for (const auto& id : order_) {
            const auto& rule = rules_.at(id);
            if (startsWith(operation, rule.operationPrefix)) {
                result.decision = rule.decision;
                result.matchedRuleId = rule.ruleId;
                result.reason = rule.reason;
                return result;
            }
        }
        return result;
    }

    std::vector<GuardrailRule> rules() const {
        std::vector<GuardrailRule> out;
        for (const auto& id : order_) out.push_back(rules_.at(id));
        return out;
    }

private:
    std::map<std::string, GuardrailRule> rules_;
    std::vector<std::string> order_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    static bool startsWith(const std::string& text, const std::string& prefix) {
        return text.rfind(prefix, 0) == 0;
    }
};
