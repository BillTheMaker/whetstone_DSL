#pragma once
// Step 330: ReviewGate — Auto-Approve Rules and Review Queue
//
// Configurable rules for auto-approving low-risk results and surfacing
// high-risk results for human review. Default policy auto-approves
// deterministic/template workers with high confidence.

#include "WorkItem.h"
#include <string>
#include <vector>

// --- AutoApproveRule ---

struct AutoApproveRule {
    std::string workerType;    // match worker type ("deterministic", "template", "*")
    int maxComplexity = 10;    // max cognitive complexity to auto-approve
    float minConfidence = 0.9f; // minimum worker confidence
    std::string riskLevel = "low"; // max risk level ("none", "low")

    json toJson() const {
        return json{
            {"workerType", workerType},
            {"maxComplexity", maxComplexity},
            {"minConfidence", minConfidence},
            {"riskLevel", riskLevel}
        };
    }

    static AutoApproveRule fromJson(const json& j) {
        AutoApproveRule r;
        if (j.contains("workerType")) r.workerType = j["workerType"].get<std::string>();
        if (j.contains("maxComplexity")) r.maxComplexity = j["maxComplexity"].get<int>();
        if (j.contains("minConfidence")) r.minConfidence = j["minConfidence"].get<float>();
        if (j.contains("riskLevel")) r.riskLevel = j["riskLevel"].get<std::string>();
        return r;
    }
};

// --- ReviewPolicy ---

struct ReviewPolicy {
    std::vector<AutoApproveRule> autoApproveRules;
    std::string defaultAction = "require-review"; // "require-review" | "auto-approve"

    json toJson() const {
        json rules = json::array();
        for (const auto& r : autoApproveRules) {
            rules.push_back(r.toJson());
        }
        return json{
            {"autoApproveRules", rules},
            {"defaultAction", defaultAction}
        };
    }

    static ReviewPolicy fromJson(const json& j) {
        ReviewPolicy p;
        if (j.contains("defaultAction")) p.defaultAction = j["defaultAction"].get<std::string>();
        if (j.contains("autoApproveRules")) {
            for (const auto& rj : j["autoApproveRules"]) {
                p.autoApproveRules.push_back(AutoApproveRule::fromJson(rj));
            }
        }
        return p;
    }

    static ReviewPolicy getDefault() {
        ReviewPolicy p;
        p.defaultAction = "require-review";
        // Auto-approve deterministic with high confidence
        AutoApproveRule detRule;
        detRule.workerType = "deterministic";
        detRule.minConfidence = 0.9f;
        detRule.riskLevel = "low";
        p.autoApproveRules.push_back(detRule);
        // Auto-approve template with high confidence
        AutoApproveRule tmplRule;
        tmplRule.workerType = "template";
        tmplRule.minConfidence = 0.9f;
        tmplRule.riskLevel = "low";
        p.autoApproveRules.push_back(tmplRule);
        return p;
    }
};

// --- ReviewDecision ---

struct ReviewDecision {
    bool approved = false;
    std::string reasoning;
    std::string ruleMatched; // which rule matched, or why not
};

// --- Risk level helpers ---

inline int riskLevelToInt(const std::string& level) {
    if (level == "none") return 0;
    if (level == "low") return 1;
    if (level == "medium") return 2;
    if (level == "high") return 3;
    return 4;
}

// --- ReviewGate ---

class ReviewGate {
public:
    ReviewDecision shouldAutoApprove(const WorkItem& item,
                                     const WorkItemResult& result,
                                     const ReviewPolicy& policy) const {
        ReviewDecision decision;

        // Explicit @Review(required) always requires review
        if (item.reviewRequired) {
            decision.approved = false;
            decision.reasoning = "Explicit @Review(required) — human review mandatory";
            decision.ruleMatched = "review-annotation-override";
            return decision;
        }

        // Check auto-approve rules
        for (const auto& rule : policy.autoApproveRules) {
            if (matchesRule(item, result, rule)) {
                decision.approved = true;
                decision.reasoning = "Auto-approved: worker=" + item.workerType +
                    " confidence=" + std::to_string(result.confidence) +
                    " matches rule for " + rule.workerType;
                decision.ruleMatched = "auto-approve:" + rule.workerType;
                return decision;
            }
        }

        // Default action
        if (policy.defaultAction == "auto-approve") {
            decision.approved = true;
            decision.reasoning = "Default policy: auto-approve";
            decision.ruleMatched = "default-auto-approve";
        } else {
            decision.approved = false;
            decision.reasoning = "No auto-approve rule matched — requires review";
            decision.ruleMatched = "default-require-review";
        }

        return decision;
    }

private:
    bool matchesRule(const WorkItem& item, const WorkItemResult& result,
                     const AutoApproveRule& rule) const {
        // Worker type must match (or wildcard)
        if (rule.workerType != "*" && rule.workerType != item.workerType) {
            return false;
        }

        // Confidence must meet minimum
        if (result.confidence < rule.minConfidence) {
            return false;
        }

        return true;
    }
};
