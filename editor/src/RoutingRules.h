#pragma once
// Step 389: RoutingRules — Configurable, composable routing rules engine
//
// Makes routing rules configurable rather than hardcoded in RoutingEngine.
// Rules have conditions (AND-combined) and actions. First matching rule wins.
// Default rules encode the same logic as RoutingEngine's hardcoded cascade.

#include "WorkItem.h"
#include "RoutingEngine.h"
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

// --- RuleCondition ---

struct RuleCondition {
    std::string type;       // "annotation" | "complexity" | "contextWidth" | "nodeType"
                            // | "rejectionCount" | "language" | "pattern"
    std::string property;   // for annotation: annotation type/property name
    std::string op;         // "==" | "!=" | "<" | ">" | "<=" | ">="
    std::string value;      // comparison target (string or numeric-as-string)

    json toJson() const {
        return json{{"type", type}, {"property", property}, {"op", op}, {"value", value}};
    }

    static RuleCondition fromJson(const json& j) {
        RuleCondition c;
        if (j.contains("type")) c.type = j["type"].get<std::string>();
        if (j.contains("property")) c.property = j["property"].get<std::string>();
        if (j.contains("op")) c.op = j["op"].get<std::string>();
        if (j.contains("value")) c.value = j["value"].get<std::string>();
        return c;
    }
};

// --- RoutingAction ---

struct RoutingAction {
    std::string workerType;             // "deterministic"|"template"|"slm"|"llm"|"human"
    bool reviewRequired = false;
    std::string contextWidthOverride;   // optional: override context width
    float budgetMultiplier = 1.0f;      // multiply token budget

    json toJson() const {
        json j = {{"workerType", workerType}, {"reviewRequired", reviewRequired},
                  {"budgetMultiplier", budgetMultiplier}};
        if (!contextWidthOverride.empty()) j["contextWidthOverride"] = contextWidthOverride;
        return j;
    }

    static RoutingAction fromJson(const json& j) {
        RoutingAction a;
        if (j.contains("workerType")) a.workerType = j["workerType"].get<std::string>();
        if (j.contains("reviewRequired")) a.reviewRequired = j["reviewRequired"].get<bool>();
        if (j.contains("contextWidthOverride")) a.contextWidthOverride = j["contextWidthOverride"].get<std::string>();
        if (j.contains("budgetMultiplier")) a.budgetMultiplier = j["budgetMultiplier"].get<float>();
        return a;
    }
};

// --- RoutingRule ---

struct RoutingRule {
    std::string name;
    int priority = 100;     // lower = higher priority
    std::vector<RuleCondition> conditions;  // all must match (AND)
    RoutingAction action;

    json toJson() const {
        json condArr = json::array();
        for (const auto& c : conditions) condArr.push_back(c.toJson());
        return json{{"name", name}, {"priority", priority},
                    {"conditions", condArr}, {"action", action.toJson()}};
    }

    static RoutingRule fromJson(const json& j) {
        RoutingRule r;
        if (j.contains("name")) r.name = j["name"].get<std::string>();
        if (j.contains("priority")) r.priority = j["priority"].get<int>();
        if (j.contains("conditions")) {
            for (const auto& c : j["conditions"]) r.conditions.push_back(RuleCondition::fromJson(c));
        }
        if (j.contains("action")) r.action = RoutingAction::fromJson(j["action"]);
        return r;
    }
};

// --- Condition evaluation helpers ---

namespace routing_rules_detail {

inline bool compareNumeric(const std::string& op, float lhs, float rhs) {
    if (op == "==" || op == "=") return lhs == rhs;
    if (op == "!=") return lhs != rhs;
    if (op == "<") return lhs < rhs;
    if (op == ">") return lhs > rhs;
    if (op == "<=") return lhs <= rhs;
    if (op == ">=") return lhs >= rhs;
    return false;
}

inline float safeFloat(const std::string& s) {
    try { return std::stof(s); } catch (...) { return 0.0f; }
}

inline bool evaluateCondition(const RuleCondition& cond, const WorkItem& item) {
    if (cond.type == "annotation") {
        // Check workerType annotation (the most common case)
        if (cond.property == "workerType" || cond.property == "Automatability") {
            if (cond.op == "==" || cond.op == "=") return item.workerType == cond.value;
            if (cond.op == "!=") return item.workerType != cond.value;
        }
        if (cond.property == "reviewRequired") {
            bool val = (cond.value == "true");
            if (cond.op == "==" || cond.op == "=") return item.reviewRequired == val;
            if (cond.op == "!=") return item.reviewRequired != val;
        }
        return false;
    }

    if (cond.type == "complexity") {
        // Complexity conditions compare against priority as a proxy
        // (priority maps: critical=0, high=1, medium=2, low=3)
        float itemVal = static_cast<float>(priorityToInt(item.priority));
        float threshold = safeFloat(cond.value);
        return compareNumeric(cond.op, itemVal, threshold);
    }

    if (cond.type == "contextWidth") {
        std::string width = item.contextWidth.empty() ? "local" : item.contextWidth;
        if (cond.op == "==" || cond.op == "=") return width == cond.value;
        if (cond.op == "!=") return width != cond.value;
        return false;
    }

    if (cond.type == "nodeType") {
        if (cond.op == "==" || cond.op == "=") return item.nodeType == cond.value;
        if (cond.op == "!=") return item.nodeType != cond.value;
        return false;
    }

    if (cond.type == "rejectionCount") {
        float count = static_cast<float>(item.rejectionHistory.size());
        float threshold = safeFloat(cond.value);
        return compareNumeric(cond.op, count, threshold);
    }

    if (cond.type == "language") {
        // Language is not directly on WorkItem, use bufferId extension as hint
        std::string ext;
        auto dot = item.bufferId.rfind('.');
        if (dot != std::string::npos) ext = item.bufferId.substr(dot + 1);
        if (cond.op == "==" || cond.op == "=") return ext == cond.value || item.bufferId == cond.value;
        if (cond.op == "!=") return ext != cond.value && item.bufferId != cond.value;
        return false;
    }

    if (cond.type == "pattern") {
        bool isGetter = isGetterSetterPattern(item.nodeName);
        bool isConstructor = (item.nodeName == "__init__" || item.nodeName == "constructor" ||
                              item.nodeName.find("Constructor") != std::string::npos);
        if (cond.value == "getter-setter") return isGetter;
        if (cond.value == "constructor") return isConstructor;
        if (cond.value == "simple") return isGetter || isConstructor;
        return false;
    }

    return false;
}

} // namespace routing_rules_detail

// --- RulesEngine ---

class RulesEngine {
public:
    void addRule(const RoutingRule& rule) {
        rules_.push_back(rule);
        sortRules();
    }

    void clearRules() { rules_.clear(); }

    size_t ruleCount() const { return rules_.size(); }

    // Evaluate: first matching rule wins
    RoutingDecision evaluate(const WorkItem& item) const {
        for (const auto& rule : rules_) {
            if (matchesAll(rule, item)) {
                return applyAction(rule, item);
            }
        }
        // Fall through to default routing
        return defaultRoute(item);
    }

    // Get the default rules (encoding the same logic as RoutingEngine's hardcoded cascade)
    static std::vector<RoutingRule> getDefaultRules() {
        std::vector<RoutingRule> rules;

        // Rule 1: Explicit worker type override (highest priority)
        {
            RoutingRule r;
            r.name = "explicit-worker";
            r.priority = 10;
            r.conditions.push_back({"annotation", "workerType", "!=", ""});
            r.action.workerType = ""; // Will be filled from item
            rules.push_back(r);
        }

        // Rule 2: Getter/setter pattern → template
        {
            RoutingRule r;
            r.name = "getter-setter-template";
            r.priority = 20;
            r.conditions.push_back({"pattern", "", "==", "getter-setter"});
            r.action.workerType = "template";
            r.action.reviewRequired = false;
            rules.push_back(r);
        }

        // Rule 3: Cross-project context → LLM
        {
            RoutingRule r;
            r.name = "cross-project-llm";
            r.priority = 30;
            r.conditions.push_back({"contextWidth", "", "==", "cross-project"});
            r.action.workerType = "llm";
            r.action.reviewRequired = false;
            rules.push_back(r);
        }

        // Rule 4: Project context → LLM
        {
            RoutingRule r;
            r.name = "project-context-llm";
            r.priority = 40;
            r.conditions.push_back({"contextWidth", "", "==", "project"});
            r.action.workerType = "llm";
            r.action.reviewRequired = false;
            rules.push_back(r);
        }

        // Rule 5: Rejection escalation (>=1 rejection → escalate)
        {
            RoutingRule r;
            r.name = "rejection-escalation";
            r.priority = 50;
            r.conditions.push_back({"rejectionCount", "", ">=", "1"});
            r.action.workerType = "llm"; // escalated
            r.action.reviewRequired = true;
            r.action.budgetMultiplier = 1.5f;
            rules.push_back(r);
        }

        // Rule 6: Local context default → SLM
        {
            RoutingRule r;
            r.name = "local-default-slm";
            r.priority = 90;
            r.conditions.push_back({"contextWidth", "", "==", "local"});
            r.action.workerType = "slm";
            r.action.reviewRequired = false;
            rules.push_back(r);
        }

        // Rule 7: File context default → SLM
        {
            RoutingRule r;
            r.name = "file-default-slm";
            r.priority = 91;
            r.conditions.push_back({"contextWidth", "", "==", "file"});
            r.action.workerType = "slm";
            r.action.reviewRequired = false;
            rules.push_back(r);
        }

        return rules;
    }

    // Load/save rules from JSON
    json saveRules() const {
        json arr = json::array();
        for (const auto& r : rules_) arr.push_back(r.toJson());
        return arr;
    }

    void loadRules(const json& j) {
        rules_.clear();
        if (j.is_array()) {
            for (const auto& elem : j) {
                rules_.push_back(RoutingRule::fromJson(elem));
            }
            sortRules();
        }
    }

    const std::vector<RoutingRule>& getRules() const { return rules_; }

private:
    std::vector<RoutingRule> rules_;

    void sortRules() {
        std::sort(rules_.begin(), rules_.end(),
                  [](const RoutingRule& a, const RoutingRule& b) {
                      return a.priority < b.priority;
                  });
    }

    static bool matchesAll(const RoutingRule& rule, const WorkItem& item) {
        if (rule.conditions.empty()) return true;
        for (const auto& cond : rule.conditions) {
            if (!routing_rules_detail::evaluateCondition(cond, item)) return false;
        }
        return true;
    }

    static RoutingDecision applyAction(const RoutingRule& rule, const WorkItem& item) {
        RoutingDecision d;
        // Special case: explicit-worker rule uses item's own worker type
        if (rule.name == "explicit-worker" && !item.workerType.empty()) {
            d.workerType = item.workerType;
        } else {
            d.workerType = rule.action.workerType;
        }
        d.reviewRequired = rule.action.reviewRequired;
        d.contextWidth = rule.action.contextWidthOverride.empty()
            ? (item.contextWidth.empty() ? "local" : item.contextWidth)
            : rule.action.contextWidthOverride;
        d.contextBudgetTokens = static_cast<int>(
            estimateContextBudget(d.contextWidth) * rule.action.budgetMultiplier);
        d.confidence = 0.85f;
        d.reasoning = "Matched rule: " + rule.name;
        d.agentRole = agentRoleForWorker(d.workerType);
        return d;
    }

    static RoutingDecision defaultRoute(const WorkItem& item) {
        RoutingDecision d;
        d.contextWidth = item.contextWidth.empty() ? "local" : item.contextWidth;
        d.contextBudgetTokens = estimateContextBudget(d.contextWidth);
        d.workerType = "llm";
        d.reviewRequired = false;
        d.confidence = 0.5f;
        d.reasoning = "Default routing to LLM (no rule matched)";
        d.agentRole = "generator";
        return d;
    }
};
