#pragma once
// Step 326: RoutingEngine — Annotation-to-Dispatch Logic
//
// Given a WorkItem with routing annotations, produces a RoutingDecision
// that says what kind of worker should handle it and how much context
// to provide. Rules follow a priority cascade from explicit annotations
// through complexity and context width heuristics to pattern defaults.

#include "WorkItem.h"
#include <string>
#include <vector>
#include <algorithm>

// --- RoutingDecision ---

struct RoutingDecision {
    std::string workerType;          // "deterministic" | "template" | "slm" | "llm" | "human"
    std::string contextWidth;        // "local" | "file" | "project" | "cross-project"
    int contextBudgetTokens = 0;     // estimated tokens for context window
    bool reviewRequired = false;     // whether result needs human review
    float confidence = 0.0f;         // engine's confidence in this routing (0.0-1.0)
    std::string reasoning;           // human-readable explanation
    std::string agentRole;           // "linter" | "refactor" | "generator"

    json toJson() const {
        return json{
            {"workerType", workerType},
            {"contextWidth", contextWidth},
            {"contextBudgetTokens", contextBudgetTokens},
            {"reviewRequired", reviewRequired},
            {"confidence", confidence},
            {"reasoning", reasoning},
            {"agentRole", agentRole}
        };
    }

    static RoutingDecision fromJson(const json& j) {
        RoutingDecision d;
        if (j.contains("workerType")) d.workerType = j["workerType"].get<std::string>();
        if (j.contains("contextWidth")) d.contextWidth = j["contextWidth"].get<std::string>();
        if (j.contains("contextBudgetTokens")) d.contextBudgetTokens = j["contextBudgetTokens"].get<int>();
        if (j.contains("reviewRequired")) d.reviewRequired = j["reviewRequired"].get<bool>();
        if (j.contains("confidence")) d.confidence = j["confidence"].get<float>();
        if (j.contains("reasoning")) d.reasoning = j["reasoning"].get<std::string>();
        if (j.contains("agentRole")) d.agentRole = j["agentRole"].get<std::string>();
        return d;
    }
};

// --- Context budget estimation ---

inline int estimateContextBudget(const std::string& contextWidth) {
    if (contextWidth == "local") return 500;
    if (contextWidth == "file") return 2000;
    if (contextWidth == "project") return 8000;
    if (contextWidth == "cross-project") return 16000;
    return 500; // default to local
}

// --- Agent role from worker type ---

inline std::string agentRoleForWorker(const std::string& workerType) {
    if (workerType == "deterministic" || workerType == "template") return "refactor";
    if (workerType == "human") return "generator";
    return "generator"; // slm, llm
}

// --- Pattern detection ---

inline bool isGetterSetterPattern(const std::string& nodeName) {
    if (nodeName.size() < 4) return false;
    std::string lower = nodeName;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower.substr(0, 3) == "get" || lower.substr(0, 3) == "set" ||
           lower.substr(0, 2) == "is";
}

// --- RoutingEngine ---

class RoutingEngine {
public:
    // Main entry point: route a single work item
    RoutingDecision route(const WorkItem& item) const {
        RoutingDecision decision;
        decision.contextWidth = item.contextWidth.empty() ? "local" : item.contextWidth;
        decision.reviewRequired = item.reviewRequired;
        decision.contextBudgetTokens = estimateContextBudget(decision.contextWidth);

        // Track how many annotations contributed to the decision
        int annotationSignals = 0;

        // Rule 1: Explicit @Automatability override
        if (!item.workerType.empty() && item.workerType != "llm") {
            // Non-default workerType means architect explicitly set it
            decision.workerType = item.workerType;
            decision.reasoning = "Explicit @Automatability(" + item.workerType + ")";
            decision.confidence = 0.95f;
            annotationSignals++;
            // Human routing always requires review
            if (item.workerType == "human") {
                decision.reviewRequired = true;
                decision.confidence = 0.9f;
                decision.reasoning = "Routed to human (high ambiguity or explicit)";
            }
            decision.agentRole = agentRoleForWorker(decision.workerType);
            return decision;
        }

        // Rule 3: Review gate
        if (item.reviewRequired) {
            annotationSignals++;
        }

        // Rule 5: Context width escalation (evaluated before complexity
        //          since we may not have complexity annotations)
        if (decision.contextWidth == "cross-project") {
            decision.workerType = "llm";
            decision.reasoning = "Cross-project context requires LLM";
            decision.confidence = 0.8f;
            annotationSignals++;
            decision.agentRole = "generator";
            return decision;
        }

        if (decision.contextWidth == "project") {
            decision.workerType = "llm";
            decision.reasoning = "Project-wide context favors LLM";
            decision.confidence = 0.75f;
            annotationSignals++;
            decision.agentRole = "generator";
            return decision;
        }

        // Rule 6: Pattern defaults (no complexity annotations available
        //          through WorkItem, so use heuristics)

        // Getter/setter/accessor → template
        if (isGetterSetterPattern(item.nodeName)) {
            decision.workerType = "template";
            decision.reasoning = "Getter/setter pattern → template";
            decision.confidence = 0.9f;
            decision.agentRole = "refactor";
            return decision;
        }

        // Context-based defaults
        if (decision.contextWidth == "local") {
            // Local context: simple enough for template or SLM
            decision.workerType = "slm";
            decision.reasoning = "Local context, default to SLM";
            decision.confidence = 0.6f;
            decision.agentRole = "generator";
        } else if (decision.contextWidth == "file") {
            decision.workerType = "slm";
            decision.reasoning = "File context, default to SLM";
            decision.confidence = 0.55f;
            decision.agentRole = "generator";
        } else {
            // Fallback
            decision.workerType = "llm";
            decision.reasoning = "Default routing to LLM";
            decision.confidence = 0.5f;
            decision.agentRole = "generator";
        }

        // Boost confidence if we have annotation signals
        if (annotationSignals > 0) {
            decision.confidence = std::min(0.95f,
                decision.confidence + 0.1f * annotationSignals);
        }

        return decision;
    }

    // Batch routing
    std::vector<RoutingDecision> routeBatch(const std::vector<WorkItem>& items) const {
        std::vector<RoutingDecision> decisions;
        decisions.reserve(items.size());
        for (const auto& item : items) {
            decisions.push_back(route(item));
        }
        return decisions;
    }

    // Route and apply: updates the WorkItem's workerType + reviewRequired
    void routeAndApply(WorkItem& item) const {
        auto decision = route(item);
        item.workerType = decision.workerType;
        item.reviewRequired = decision.reviewRequired;
        item.contextWidth = decision.contextWidth;
    }
};
