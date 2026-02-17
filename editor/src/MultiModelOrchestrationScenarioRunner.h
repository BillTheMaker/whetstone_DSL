#pragma once

// Step 502: Scenario - Multi-Model Orchestration
// End-to-end orchestration scenario with deterministic/template/slm/llm/human workers.

#include "RoutingEngine.h"

#include <map>
#include <string>
#include <vector>

enum class MultiModelTaskGroup { Deterministic, Template, SLM, LLM, Human };

struct MultiModelTaskResult {
    std::string id;
    std::string name;
    MultiModelTaskGroup group = MultiModelTaskGroup::SLM;
    WorkItem item;
    RoutingDecision route;
    bool autoCompleted = false;
    bool completed = false;
    bool reviewRequired = false;
    int estimatedTokens = 0;
    int actualTokens = 0;
};

struct MultiModelProgressPoint {
    int completed = 0;
    int total = 0;
    float percent = 0.0f;
};

struct MultiModelOrchestrationScenarioResult {
    std::vector<MultiModelTaskResult> tasks;
    std::vector<MultiModelProgressPoint> progress;
    std::map<std::string, int> routedCounts;
    int estimatedTotalTokens = 0;
    int actualTotalTokens = 0;
    int reviewGateCount = 0;
    bool reviewGatesExercised = false;
    std::vector<std::string> notes;

    int countGroup(MultiModelTaskGroup g) const {
        int n = 0;
        for (const auto& t : tasks) if (t.group == g) ++n;
        return n;
    }
};

class MultiModelOrchestrationScenarioRunner {
public:
    static MultiModelOrchestrationScenarioResult run() {
        MultiModelOrchestrationScenarioResult out;
        RoutingEngine routing;

        auto items = buildScenarioItems();
        const int total = static_cast<int>(items.size());
        int completed = 0;

        for (auto& task : items) {
            task.route = routing.route(task.item);
            task.estimatedTokens = task.route.contextBudgetTokens + estimatedOutputTokens(task.route.workerType);
            out.estimatedTotalTokens += task.estimatedTokens;
            out.routedCounts[task.route.workerType]++;

            executeTask(task, out.reviewGateCount);
            out.actualTotalTokens += task.actualTokens;
            if (task.completed) ++completed;
            out.progress.push_back({completed, total, percent(completed, total)});
        }

        out.tasks = std::move(items);
        out.reviewGatesExercised = out.reviewGateCount > 0;
        out.notes.push_back("Multi-model orchestration scenario executed with 20 tasks");
        out.notes.push_back("Routing mix includes deterministic/template/slm/llm/human paths");
        return out;
    }

private:
    static std::vector<MultiModelTaskResult> buildScenarioItems() {
        std::vector<MultiModelTaskResult> out;
        int idx = 0;
        auto makeId = [&idx]() { return "mm-" + std::to_string(++idx); };

        // 5 deterministic tasks
        out.push_back(makeTask(makeId(), "normalize_id", MultiModelTaskGroup::Deterministic, "deterministic", "local", false));
        out.push_back(makeTask(makeId(), "sanitize_email", MultiModelTaskGroup::Deterministic, "deterministic", "local", false));
        out.push_back(makeTask(makeId(), "hash_password", MultiModelTaskGroup::Deterministic, "deterministic", "local", false));
        out.push_back(makeTask(makeId(), "build_slug", MultiModelTaskGroup::Deterministic, "deterministic", "local", false));
        out.push_back(makeTask(makeId(), "validate_url", MultiModelTaskGroup::Deterministic, "deterministic", "local", false));

        // 5 template tasks (getter/setter/constructor style)
        out.push_back(makeTask(makeId(), "get_user", MultiModelTaskGroup::Template, "", "local", false));
        out.push_back(makeTask(makeId(), "set_user", MultiModelTaskGroup::Template, "", "local", false));
        out.push_back(makeTask(makeId(), "get_short_link", MultiModelTaskGroup::Template, "", "local", false));
        out.push_back(makeTask(makeId(), "set_short_link", MultiModelTaskGroup::Template, "", "local", false));
        out.push_back(makeTask(makeId(), "is_active", MultiModelTaskGroup::Template, "", "local", false));

        // 5 SLM tasks (narrow context)
        out.push_back(makeTask(makeId(), "apply_rate_limit", MultiModelTaskGroup::SLM, "", "local", false));
        out.push_back(makeTask(makeId(), "merge_query_filters", MultiModelTaskGroup::SLM, "", "file", false));
        out.push_back(makeTask(makeId(), "serialize_session", MultiModelTaskGroup::SLM, "", "local", false));
        out.push_back(makeTask(makeId(), "parse_feature_flags", MultiModelTaskGroup::SLM, "", "file", false));
        out.push_back(makeTask(makeId(), "compute_expiry", MultiModelTaskGroup::SLM, "", "local", false));

        // 3 LLM tasks (wide context + review gate)
        out.push_back(makeTask(makeId(), "cross_module_cache_invalidation", MultiModelTaskGroup::LLM, "", "project", true));
        out.push_back(makeTask(makeId(), "multi_region_failover_strategy", MultiModelTaskGroup::LLM, "", "cross-project", true));
        out.push_back(makeTask(makeId(), "consistency_resolution_policy", MultiModelTaskGroup::LLM, "", "project", true));

        // 2 human tasks (architectural decisions)
        out.push_back(makeTask(makeId(), "select_sharding_strategy", MultiModelTaskGroup::Human, "human", "cross-project", true));
        out.push_back(makeTask(makeId(), "approve_security_boundary_model", MultiModelTaskGroup::Human, "human", "project", true));

        return out;
    }

    static MultiModelTaskResult makeTask(const std::string& id,
                                         const std::string& name,
                                         MultiModelTaskGroup group,
                                         const std::string& workerType,
                                         const std::string& contextWidth,
                                         bool reviewRequired) {
        MultiModelTaskResult t;
        t.id = id;
        t.name = name;
        t.group = group;
        t.item.id = id;
        t.item.nodeName = name;
        t.item.nodeType = "Function";
        t.item.bufferId = "scenario502";
        t.item.workerType = workerType;
        t.item.contextWidth = contextWidth;
        t.item.reviewRequired = reviewRequired;
        t.item.status = WI_READY;
        return t;
    }

    static void executeTask(MultiModelTaskResult& t, int& reviewGateCount) {
        t.reviewRequired = t.route.reviewRequired;
        if (t.route.workerType == "deterministic" || t.route.workerType == "template") {
            t.autoCompleted = true;
            t.completed = true;
            t.actualTokens = 0;
            return;
        }

        if (t.route.workerType == "slm") {
            t.completed = true;
            t.actualTokens = t.route.contextBudgetTokens / 3;
            return;
        }

        if (t.route.workerType == "llm") {
            t.completed = true;
            t.actualTokens = static_cast<int>(t.route.contextBudgetTokens * 0.85f);
            if (t.reviewRequired) ++reviewGateCount;
            return;
        }

        // Human path
        t.completed = true;
        t.actualTokens = 0;
        ++reviewGateCount;
    }

    static int estimatedOutputTokens(const std::string& workerType) {
        if (workerType == "deterministic" || workerType == "template" || workerType == "human") return 0;
        if (workerType == "slm") return 250;
        return 900;
    }

    static float percent(int done, int total) {
        if (total <= 0) return 0.0f;
        return 100.0f * static_cast<float>(done) / static_cast<float>(total);
    }
};
