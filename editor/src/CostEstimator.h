#pragma once
// Step 390: CostEstimator — Token cost estimation and optimization suggestions
//
// Before executing a workflow, estimate the total cost in tokens and suggest
// optimizations (narrowing context, using templates, batching related tasks).

#include "WorkflowState.h"
#include "RoutingEngine.h"
#include "RoutingRules.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

// --- OptimizationSuggestion ---

struct OptimizationSuggestion {
    std::string description;
    int tokensSaved = 0;
    std::vector<std::string> itemIds;
    std::string action;  // "narrow-context" | "use-template" | "batch" | "skip-review"

    json toJson() const {
        return json{{"description", description}, {"tokensSaved", tokensSaved},
                    {"itemIds", itemIds}, {"action", action}};
    }

    static OptimizationSuggestion fromJson(const json& j) {
        OptimizationSuggestion s;
        if (j.contains("description")) s.description = j["description"].get<std::string>();
        if (j.contains("tokensSaved")) s.tokensSaved = j["tokensSaved"].get<int>();
        if (j.contains("itemIds") && j["itemIds"].is_array()) {
            for (const auto& id : j["itemIds"]) s.itemIds.push_back(id.get<std::string>());
        }
        if (j.contains("action")) s.action = j["action"].get<std::string>();
        return s;
    }
};

// --- CostEstimate ---

struct CostEstimate {
    int totalContextTokens = 0;
    int totalOutputTokens = 0;
    std::map<std::string, int> byWorkerType;  // tokens per worker type
    int deterministicTasks = 0;
    float estimatedCost = 0.0f;  // rough USD estimate
    std::vector<OptimizationSuggestion> optimizationSuggestions;

    json toJson() const {
        json sugArr = json::array();
        for (const auto& s : optimizationSuggestions) sugArr.push_back(s.toJson());
        return json{
            {"totalContextTokens", totalContextTokens},
            {"totalOutputTokens", totalOutputTokens},
            {"byWorkerType", byWorkerType},
            {"deterministicTasks", deterministicTasks},
            {"estimatedCost", estimatedCost},
            {"optimizationSuggestions", sugArr}
        };
    }

    static CostEstimate fromJson(const json& j) {
        CostEstimate e;
        if (j.contains("totalContextTokens")) e.totalContextTokens = j["totalContextTokens"].get<int>();
        if (j.contains("totalOutputTokens")) e.totalOutputTokens = j["totalOutputTokens"].get<int>();
        if (j.contains("byWorkerType") && j["byWorkerType"].is_object()) {
            for (auto& [k, v] : j["byWorkerType"].items()) e.byWorkerType[k] = v.get<int>();
        }
        if (j.contains("deterministicTasks")) e.deterministicTasks = j["deterministicTasks"].get<int>();
        if (j.contains("estimatedCost")) e.estimatedCost = j["estimatedCost"].get<float>();
        if (j.contains("optimizationSuggestions") && j["optimizationSuggestions"].is_array()) {
            for (const auto& s : j["optimizationSuggestions"])
                e.optimizationSuggestions.push_back(OptimizationSuggestion::fromJson(s));
        }
        return e;
    }
};

// --- CostEstimator ---

// Collect all items from a TaskQueue across all statuses
inline std::vector<WorkItem> collectAllItems(const TaskQueue& q) {
    std::vector<WorkItem> all;
    for (const auto& status : {WI_PENDING, WI_READY, WI_ASSIGNED, WI_IN_PROGRESS,
                                WI_REVIEW, WI_COMPLETE, WI_REJECTED}) {
        auto items = q.getByStatus(status);
        all.insert(all.end(), items.begin(), items.end());
    }
    return all;
}

class CostEstimator {
public:
    // Estimate cost for entire workflow using the routing engine
    CostEstimate estimate(const WorkflowState& workflow, const RoutingEngine& routing) const {
        CostEstimate est;

        auto allItems = collectAllItems(workflow.queue);
        for (const auto& item : allItems) {
            if (item.status == WI_COMPLETE) continue;

            RoutingDecision decision = routing.route(item);
            int contextTokens = decision.contextBudgetTokens;
            int outputTokens = estimateOutputTokens(decision);

            est.totalContextTokens += contextTokens;
            est.totalOutputTokens += outputTokens;
            est.byWorkerType[decision.workerType] += contextTokens + outputTokens;

            if (decision.workerType == "deterministic" || decision.workerType == "template") {
                est.deterministicTasks++;
            }
        }

        // Rough cost: $3/M input, $15/M output (Claude-class pricing)
        est.estimatedCost = (est.totalContextTokens * 3.0f / 1000000.0f) +
                            (est.totalOutputTokens * 15.0f / 1000000.0f);

        est.optimizationSuggestions = suggest(workflow, routing);
        return est;
    }

    // Generate optimization suggestions
    std::vector<OptimizationSuggestion> suggest(const WorkflowState& workflow,
                                                 const RoutingEngine& routing) const {
        std::vector<OptimizationSuggestion> suggestions;
        auto allItems = collectAllItems(workflow.queue);

        // Suggestion 1: Getter/setter functions that could use template worker
        {
            std::vector<std::string> getterIds;
            int savedTokens = 0;
            for (const auto& item : allItems) {
                if (item.status == WI_COMPLETE) continue;
                if (isGetterSetterPattern(item.nodeName)) {
                    auto decision = routing.route(item);
                    if (decision.workerType != "template" && decision.workerType != "deterministic") {
                        getterIds.push_back(item.id);
                        savedTokens += decision.contextBudgetTokens;
                    }
                }
            }
            if (!getterIds.empty()) {
                OptimizationSuggestion s;
                s.description = std::to_string(getterIds.size()) +
                    " getter/setter functions could use template worker";
                s.tokensSaved = savedTokens;
                s.itemIds = getterIds;
                s.action = "use-template";
                suggestions.push_back(s);
            }
        }

        // Suggestion 2: File/project-width tasks that could be narrowed to local
        {
            std::vector<std::string> narrowIds;
            int savedTokens = 0;
            for (const auto& item : allItems) {
                if (item.status == WI_COMPLETE) continue;
                std::string width = item.contextWidth.empty() ? "local" : item.contextWidth;
                if (width == "file" || width == "project") {
                    // Simple functions with local names could be narrowed
                    if (isGetterSetterPattern(item.nodeName) ||
                        item.nodeName.find("simple") != std::string::npos) {
                        narrowIds.push_back(item.id);
                        int current = estimateContextBudget(width);
                        int narrowed = estimateContextBudget("local");
                        savedTokens += (current - narrowed);
                    }
                }
            }
            if (!narrowIds.empty()) {
                OptimizationSuggestion s;
                s.description = std::to_string(narrowIds.size()) +
                    " tasks could be narrowed to local context";
                s.tokensSaved = savedTokens;
                s.itemIds = narrowIds;
                s.action = "narrow-context";
                suggestions.push_back(s);
            }
        }

        // Suggestion 3: Batch related tasks (same buffer, same context width)
        {
            std::map<std::string, std::vector<std::string>> groups;
            for (const auto& item : allItems) {
                if (item.status == WI_COMPLETE) continue;
                std::string width = item.contextWidth.empty() ? "local" : item.contextWidth;
                if (width == "project" || width == "cross-project") {
                    std::string key = item.bufferId + ":" + width;
                    groups[key].push_back(item.id);
                }
            }
            for (const auto& [key, ids] : groups) {
                if (ids.size() >= 2) {
                    std::string width = key.substr(key.rfind(':') + 1);
                    int sharedCost = estimateContextBudget(width);
                    int saved = sharedCost * (static_cast<int>(ids.size()) - 1);
                    OptimizationSuggestion s;
                    s.description = std::to_string(ids.size()) +
                        " related tasks could share " + width + " context";
                    s.tokensSaved = saved;
                    s.itemIds = ids;
                    s.action = "batch";
                    suggestions.push_back(s);
                }
            }
        }

        return suggestions;
    }

private:
    static int estimateOutputTokens(const RoutingDecision& decision) {
        // Deterministic/template workers produce no LLM output tokens
        if (decision.workerType == "deterministic" || decision.workerType == "template") return 0;
        // Human workers: zero automated token cost
        if (decision.workerType == "human") return 0;
        // SLM: smaller output
        if (decision.workerType == "slm") return decision.contextBudgetTokens / 2;
        // LLM: proportional to context
        return decision.contextBudgetTokens;
    }
};
