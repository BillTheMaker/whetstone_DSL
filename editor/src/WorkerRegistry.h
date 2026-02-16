#pragma once
// Step 327: WorkerRegistry — Worker Abstractions
//
// Defines the worker interface and provides concrete implementations:
// DeterministicWorker, TemplateWorker, SLMAgentWorker, LLMAgentWorker,
// HumanWorker. LLM/SLM workers prepare context bundles — actual model
// invocation happens externally via MCP.

#include "WorkItem.h"
#include "RoutingEngine.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

// --- WorkerContext: context bundle for worker execution ---

struct WorkerContext {
    json nodeAst;                     // the target node's AST
    std::string bufferContent;        // current buffer text (file+ context)
    json projectSummary;              // compact AST summaries (project+ context)
    std::vector<json> annotations;    // annotations on the target node
    std::vector<json> diagnostics;    // relevant diagnostics
    std::string skeletonIntent;       // @Intent annotation value
    std::string feedbackFromRejection; // if re-routed rejected item

    json toJson() const {
        json j;
        j["nodeAst"] = nodeAst;
        j["bufferContent"] = bufferContent;
        j["projectSummary"] = projectSummary;
        j["annotations"] = annotations;
        j["diagnostics"] = diagnostics;
        j["skeletonIntent"] = skeletonIntent;
        j["feedbackFromRejection"] = feedbackFromRejection;
        return j;
    }
};

// --- WorkerInterface (abstract base) ---

class WorkerInterface {
public:
    virtual ~WorkerInterface() = default;
    virtual std::string type() const = 0;
    virtual bool canHandle(const WorkItem& item) const = 0;
    virtual WorkItemResult execute(const WorkItem& item,
                                    const WorkerContext& context) = 0;
};

// --- DeterministicWorker ---

class DeterministicWorker : public WorkerInterface {
public:
    std::string type() const override { return "deterministic"; }

    bool canHandle(const WorkItem& item) const override {
        // Handles simple skeletons with local context
        return item.contextWidth == "local" || item.contextWidth.empty();
    }

    WorkItemResult execute(const WorkItem& item,
                            const WorkerContext& context) override {
        WorkItemResult result;

        // Use skeleton intent if available
        if (!context.skeletonIntent.empty()) {
            result.generatedCode = "// Generated from intent: " +
                                   context.skeletonIntent;
            result.confidence = 0.9f;
            result.reasoning = "Pattern-matched from @Intent annotation";
        } else {
            // Heuristic: generate a stub based on node name
            result.generatedCode = "// TODO: implement " + item.nodeName;
            result.confidence = 0.5f;
            result.reasoning = "Heuristic stub — no @Intent annotation";
        }

        result.tokensGenerated = static_cast<int>(result.generatedCode.size() / 4);
        result.tokensBudget = estimateContextBudget(item.contextWidth);
        return result;
    }
};

// --- TemplateWorker ---

class TemplateWorker : public WorkerInterface {
public:
    std::string type() const override { return "template"; }

    bool canHandle(const WorkItem& item) const override {
        return isGetterSetterPattern(item.nodeName);
    }

    WorkItemResult execute(const WorkItem& item,
                            const WorkerContext& context) override {
        WorkItemResult result;
        std::string lower = item.nodeName;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.substr(0, 3) == "get") {
            std::string field = item.nodeName.substr(3);
            // Lowercase first char of field
            if (!field.empty()) field[0] = std::tolower(field[0]);
            result.generatedCode = "return self." + field;
            result.confidence = 0.95f;
            result.reasoning = "Getter template for field '" + field + "'";
        } else if (lower.substr(0, 3) == "set") {
            std::string field = item.nodeName.substr(3);
            if (!field.empty()) field[0] = std::tolower(field[0]);
            result.generatedCode = "self." + field + " = value";
            result.confidence = 0.95f;
            result.reasoning = "Setter template for field '" + field + "'";
        } else if (lower.substr(0, 2) == "is") {
            std::string field = item.nodeName.substr(2);
            if (!field.empty()) field[0] = std::tolower(field[0]);
            result.generatedCode = "return self." + field;
            result.confidence = 0.95f;
            result.reasoning = "Boolean accessor template for field '" + field + "'";
        } else {
            result.generatedCode = "// template: " + item.nodeName;
            result.confidence = 0.5f;
            result.reasoning = "No matching template pattern";
        }

        result.tokensGenerated = static_cast<int>(result.generatedCode.size() / 4);
        result.tokensBudget = estimateContextBudget(item.contextWidth);
        return result;
    }
};

// --- SLMAgentWorker ---

class SLMAgentWorker : public WorkerInterface {
public:
    std::string type() const override { return "slm"; }

    bool canHandle(const WorkItem& item) const override {
        return item.contextWidth == "local" || item.contextWidth == "file";
    }

    WorkItemResult execute(const WorkItem& item,
                            const WorkerContext& context) override {
        WorkItemResult result;
        // Prepare context bundle — actual model invocation is external
        result.generatedCode = ""; // deferred to external SLM
        result.confidence = 0.0f;
        result.tokensBudget = estimateContextBudget(item.contextWidth);
        result.tokensGenerated = 0;

        json contextBundle = context.toJson();
        contextBundle["workerType"] = "slm";
        contextBundle["itemId"] = item.id;
        contextBundle["nodeName"] = item.nodeName;
        result.astJson = contextBundle;
        result.reasoning = "Context prepared for SLM invocation via MCP";

        if (!context.feedbackFromRejection.empty()) {
            result.reasoning += " (re-routed with feedback: " +
                                context.feedbackFromRejection + ")";
        }
        return result;
    }
};

// --- LLMAgentWorker ---

class LLMAgentWorker : public WorkerInterface {
public:
    std::string type() const override { return "llm"; }

    bool canHandle(const WorkItem& /*item*/) const override {
        return true; // LLM can handle anything
    }

    WorkItemResult execute(const WorkItem& item,
                            const WorkerContext& context) override {
        WorkItemResult result;
        result.generatedCode = ""; // deferred to external LLM
        result.confidence = 0.0f;
        result.tokensBudget = estimateContextBudget(item.contextWidth);
        result.tokensGenerated = 0;

        json contextBundle = context.toJson();
        contextBundle["workerType"] = "llm";
        contextBundle["itemId"] = item.id;
        contextBundle["nodeName"] = item.nodeName;
        result.astJson = contextBundle;
        result.reasoning = "Context prepared for LLM invocation via MCP";

        if (!context.feedbackFromRejection.empty()) {
            result.reasoning += " (re-routed with feedback: " +
                                context.feedbackFromRejection + ")";
        }
        return result;
    }
};

// --- HumanWorker ---

class HumanWorker : public WorkerInterface {
public:
    std::string type() const override { return "human"; }

    bool canHandle(const WorkItem& /*item*/) const override {
        return true; // human can handle anything
    }

    WorkItemResult execute(const WorkItem& item,
                            const WorkerContext& context) override {
        WorkItemResult result;
        result.generatedCode = ""; // deferred to human
        result.confidence = 0.0f;
        result.tokensBudget = 0;
        result.tokensGenerated = 0;

        json contextBundle = context.toJson();
        contextBundle["workerType"] = "human";
        contextBundle["itemId"] = item.id;
        contextBundle["nodeName"] = item.nodeName;
        contextBundle["status"] = "needs-human";
        result.astJson = contextBundle;
        result.reasoning = "Routed to human review — requires manual implementation";

        return result;
    }
};

// --- WorkerRegistry ---

class WorkerRegistry {
public:
    void registerWorker(std::unique_ptr<WorkerInterface> worker) {
        std::string t = worker->type();
        workers_[t] = std::move(worker);
    }

    WorkerInterface* getWorker(const std::string& workerType) const {
        auto it = workers_.find(workerType);
        if (it != workers_.end()) return it->second.get();
        return nullptr;
    }

    std::vector<std::string> registeredTypes() const {
        std::vector<std::string> types;
        for (const auto& [k, _] : workers_) {
            types.push_back(k);
        }
        return types;
    }

    static WorkerRegistry getDefaultRegistry() {
        WorkerRegistry reg;
        reg.registerWorker(std::make_unique<DeterministicWorker>());
        reg.registerWorker(std::make_unique<TemplateWorker>());
        reg.registerWorker(std::make_unique<SLMAgentWorker>());
        reg.registerWorker(std::make_unique<LLMAgentWorker>());
        reg.registerWorker(std::make_unique<HumanWorker>());
        return reg;
    }

private:
    std::map<std::string, std::unique_ptr<WorkerInterface>> workers_;
};
