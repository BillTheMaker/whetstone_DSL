#pragma once
// Step 391: WorkflowTemplates — Pre-built workflow templates for common patterns
//
// Templates: CRUD API, Module Refactor, Cross-Language Port, Test Suite Generation,
// Legacy Modernization. Each creates a populated WorkflowState with work items,
// annotations, and routing hints.

#include "WorkflowState.h"
#include "RoutingEngine.h"
#include <string>
#include <vector>
#include <map>

// --- TemplateInfo ---

struct TemplateInfo {
    std::string name;
    std::string description;
    std::vector<std::string> requiredParams;  // param names the template needs
    std::vector<std::string> optionalParams;

    json toJson() const {
        return json{{"name", name}, {"description", description},
                    {"requiredParams", requiredParams}, {"optionalParams", optionalParams}};
    }
};

// --- TemplateParam ---

struct TemplateParams {
    std::string entityName;         // for CRUD: entity name
    std::string sourceLanguage;     // for port: source language
    std::string targetLanguage;     // for port: target language
    std::string bufferId;           // buffer context
    std::vector<std::string> functionNames;  // specific functions to include
    int functionCount = 5;          // default function count when names not given

    static TemplateParams fromJson(const json& j) {
        TemplateParams p;
        if (j.contains("entityName")) p.entityName = j["entityName"].get<std::string>();
        if (j.contains("sourceLanguage")) p.sourceLanguage = j["sourceLanguage"].get<std::string>();
        if (j.contains("targetLanguage")) p.targetLanguage = j["targetLanguage"].get<std::string>();
        if (j.contains("bufferId")) p.bufferId = j["bufferId"].get<std::string>();
        if (j.contains("functionNames") && j["functionNames"].is_array()) {
            for (const auto& f : j["functionNames"]) p.functionNames.push_back(f.get<std::string>());
        }
        if (j.contains("functionCount")) p.functionCount = j["functionCount"].get<int>();
        return p;
    }
};

// --- WorkflowTemplates ---

class WorkflowTemplates {
public:
    static std::vector<TemplateInfo> getTemplates() {
        return {
            {"crud-api",
             "Generate CRUD operations (create, read, update, delete) for an entity. "
             "Deterministic routing for getters/setters, LLM for business logic, human review for security.",
             {"entityName"}, {"bufferId", "functionCount"}},

            {"module-refactor",
             "Refactor existing code: extract functions, rename, restructure. "
             "Mostly LLM with human review for architectural decisions.",
             {"functionNames"}, {"bufferId"}},

            {"cross-language-port",
             "Port code from one language to another. Template workers for simple functions, "
             "LLM for complex logic. Creates skeleton in target language.",
             {"sourceLanguage", "targetLanguage"}, {"functionNames", "bufferId"}},

            {"test-suite",
             "Generate test functions with @Intent annotations. SLM for simple unit tests, "
             "LLM for integration tests.",
             {"functionNames"}, {"bufferId"}},

            {"legacy-modernization",
             "Update old code: replace deprecated patterns, add safety annotations, "
             "modernize idioms. Mix of deterministic and LLM workers.",
             {"functionNames"}, {"bufferId"}}
        };
    }

    static WorkflowState applyTemplate(const std::string& templateName,
                                        const TemplateParams& params) {
        std::string buf = params.bufferId.empty() ? "main.py" : params.bufferId;
        WorkflowState wf(templateName + "-" + (params.entityName.empty() ? "project" : params.entityName));

        if (templateName == "crud-api") return buildCrud(wf, params, buf);
        if (templateName == "module-refactor") return buildRefactor(wf, params, buf);
        if (templateName == "cross-language-port") return buildPort(wf, params, buf);
        if (templateName == "test-suite") return buildTestSuite(wf, params, buf);
        if (templateName == "legacy-modernization") return buildModernize(wf, params, buf);

        return wf; // empty for unknown template
    }

    static bool isValidTemplate(const std::string& name) {
        for (const auto& t : getTemplates()) {
            if (t.name == name) return true;
        }
        return false;
    }

private:
    static void enqueueItem(WorkflowState& wf, const std::string& id,
                            const std::string& name, const std::string& nodeType,
                            const std::string& workerType, const std::string& contextWidth,
                            const std::string& priority, bool reviewRequired,
                            const std::string& bufferId,
                            const std::vector<std::string>& deps = {}) {
        WorkItem item;
        item.id = id;
        item.nodeId = id + "_node";
        item.nodeName = name;
        item.nodeType = nodeType;
        item.bufferId = bufferId;
        item.workerType = workerType;
        item.contextWidth = contextWidth;
        item.priority = priority;
        item.reviewRequired = reviewRequired;
        item.status = WI_PENDING;
        item.createdAt = workItemTimestamp();
        item.dependencies = deps;
        wf.queue.enqueue(item);
    }

    static WorkflowState buildCrud(WorkflowState& wf, const TemplateParams& params,
                                    const std::string& buf) {
        std::string e = params.entityName.empty() ? "Entity" : params.entityName;

        // Create — LLM (validation logic)
        enqueueItem(wf, "crud-create", "create" + e, "Function", "llm", "file", "high", true, buf);
        // Read — template (simple getter)
        enqueueItem(wf, "crud-read", "get" + e, "Function", "template", "local", "medium", false, buf);
        // Read all — SLM
        enqueueItem(wf, "crud-list", "getAll" + e + "s", "Function", "slm", "file", "medium", false, buf);
        // Update — LLM (validation)
        enqueueItem(wf, "crud-update", "update" + e, "Function", "llm", "file", "high", true, buf,
                    {"crud-create"});
        // Delete — LLM (security-sensitive)
        enqueueItem(wf, "crud-delete", "delete" + e, "Function", "llm", "file", "critical", true, buf);

        return wf;
    }

    static WorkflowState buildRefactor(WorkflowState& wf, const TemplateParams& params,
                                        const std::string& buf) {
        auto names = params.functionNames;
        if (names.empty()) {
            for (int i = 0; i < params.functionCount; ++i)
                names.push_back("refactorTarget" + std::to_string(i));
        }

        for (size_t i = 0; i < names.size(); ++i) {
            std::string id = "refactor-" + std::to_string(i);
            enqueueItem(wf, id, names[i], "Function", "llm", "file", "high", true, buf);
        }
        return wf;
    }

    static WorkflowState buildPort(WorkflowState& wf, const TemplateParams& params,
                                    const std::string& buf) {
        auto names = params.functionNames;
        if (names.empty()) {
            for (int i = 0; i < params.functionCount; ++i)
                names.push_back("portFunc" + std::to_string(i));
        }

        for (size_t i = 0; i < names.size(); ++i) {
            std::string id = "port-" + std::to_string(i);
            bool isSimple = isGetterSetterPattern(names[i]);
            std::string worker = isSimple ? "template" : "llm";
            std::string ctx = isSimple ? "local" : "project";
            enqueueItem(wf, id, names[i], "Function", worker, ctx, "medium", !isSimple, buf);
        }
        return wf;
    }

    static WorkflowState buildTestSuite(WorkflowState& wf, const TemplateParams& params,
                                         const std::string& buf) {
        auto names = params.functionNames;
        if (names.empty()) {
            for (int i = 0; i < params.functionCount; ++i)
                names.push_back("testFunc" + std::to_string(i));
        }

        for (size_t i = 0; i < names.size(); ++i) {
            std::string id = "test-" + std::to_string(i);
            // Simple tests → SLM, complex → LLM
            bool isComplex = (names[i].find("integration") != std::string::npos ||
                              names[i].find("complex") != std::string::npos);
            std::string worker = isComplex ? "llm" : "slm";
            std::string ctx = isComplex ? "project" : "file";
            enqueueItem(wf, id, "test_" + names[i], "Function", worker, ctx, "medium", false, buf);
        }
        return wf;
    }

    static WorkflowState buildModernize(WorkflowState& wf, const TemplateParams& params,
                                         const std::string& buf) {
        auto names = params.functionNames;
        if (names.empty()) {
            for (int i = 0; i < params.functionCount; ++i)
                names.push_back("legacyFunc" + std::to_string(i));
        }

        for (size_t i = 0; i < names.size(); ++i) {
            std::string id = "modernize-" + std::to_string(i);
            // Mix of deterministic (simple renames) and LLM (pattern updates)
            std::string worker = (i % 3 == 0) ? "deterministic" : "llm";
            enqueueItem(wf, id, names[i], "Function", worker, "file", "medium", worker == "llm", buf);
        }
        return wf;
    }
};
