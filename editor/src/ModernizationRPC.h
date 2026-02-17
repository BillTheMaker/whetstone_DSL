#pragma once

// Step 442: Modernization RPC + MCP tool definitions.
// Provides JSON-RPC method handlers and MCP tool metadata for the
// legacy analysis → safety audit → modernization suggestion → workflow pipeline.

#include "LegacyIdiomDetector.h"
#include "SafetyAuditor.h"
#include "ModernizationSuggester.h"
#include "ModernizationWorkflow.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

// --- JSON serialization helpers ---

inline json findingToJson(const LegacyIdiomFinding& f) {
    return {{"pattern", f.pattern}, {"detail", f.detail}, {"score", f.score}};
}

inline json safetyFindingToJson(const SafetyFinding& f) {
    return {{"category", f.category}, {"annotation", f.annotation},
            {"detail", f.detail}, {"cwe", f.cwe}, {"riskLevel", f.riskLevel}};
}

inline json suggestionToJson(const ModernizeSuggestion& s) {
    std::string effort = "quick_win";
    if (s.effort == ModernizeEffort::Moderate) effort = "moderate";
    if (s.effort == ModernizeEffort::DeepRefactor) effort = "deep_refactor";
    return {{"from", s.fromPattern}, {"to", s.toReplacement},
            {"annotation", s.annotation}, {"risk", s.risk},
            {"effort", effort}, {"rationale", s.rationale}};
}

inline json workItemToJson(const ModernizeWorkItem& item) {
    std::string routing = "deterministic";
    if (item.routing == WorkItemRouting::LLM) routing = "llm";
    if (item.routing == WorkItemRouting::Human) routing = "human";
    std::string effort = "quick_win";
    if (item.effort == ModernizeEffort::Moderate) effort = "moderate";
    if (item.effort == ModernizeEffort::DeepRefactor) effort = "deep_refactor";
    return {{"id", item.id}, {"title", item.title},
            {"from", item.fromPattern}, {"to", item.toReplacement},
            {"annotation", item.annotation}, {"routing", routing},
            {"effort", effort}, {"priority", item.priority},
            {"dependsOn", item.dependsOn}};
}

// --- RPC method handlers ---

class ModernizationRPCHandler {
public:
    // analyzeLegacy: run legacy idiom detection on source
    static json handleAnalyzeLegacy(const json& params) {
        std::string source = params.value("source", "");
        std::string language = params.value("language", "c");
        std::string filePath = params.value("filePath", "unknown");

        auto report = LegacyIdiomDetector::analyzeFile(source, language, filePath);

        json findings = json::array();
        for (const auto& f : report.findings)
            findings.push_back(findingToJson(f));

        json fnFindings = json::array();
        for (const auto& ff : report.functionFindings) {
            json ffs = json::array();
            for (const auto& f : ff.findings)
                ffs.push_back(findingToJson(f));
            fnFindings.push_back({{"functionName", ff.functionName},
                                  {"findings", ffs},
                                  {"legacyScore", ff.legacyScore}});
        }

        return {{"filePath", report.filePath},
                {"language", report.language},
                {"languageVersion", report.languageVersion},
                {"legacyScore", report.legacyScore},
                {"findings", findings},
                {"functionFindings", fnFindings}};
    }

    // getSafetyReport: run safety audit on source
    static json handleGetSafetyReport(const json& params) {
        std::string source = params.value("source", "");
        std::string language = params.value("language", "c");
        std::string filePath = params.value("filePath", "unknown");

        auto report = SafetyAuditor::audit(source, language, filePath);

        json findings = json::array();
        for (const auto& f : report.findings)
            findings.push_back(safetyFindingToJson(f));

        json fnFindings = json::array();
        for (const auto& ff : report.functionFindings) {
            json ffs = json::array();
            for (const auto& f : ff.findings)
                ffs.push_back(safetyFindingToJson(f));
            fnFindings.push_back({{"functionName", ff.functionName},
                                  {"findings", ffs},
                                  {"maxRisk", ff.maxRisk}});
        }

        return {{"filePath", report.filePath},
                {"language", report.language},
                {"overallRisk", report.overallRisk},
                {"findings", findings},
                {"functionFindings", fnFindings}};
    }

    // suggestModernization: generate modernization suggestions
    static json handleSuggestModernization(const json& params) {
        std::string source = params.value("source", "");
        std::string language = params.value("language", "c");
        std::string filePath = params.value("filePath", "unknown");
        std::string targetLang = params.value("targetLanguage", "");

        auto legacy = LegacyIdiomDetector::analyzeFile(source, language, filePath);
        auto safety = SafetyAuditor::audit(source, language, filePath);
        auto report = ModernizationSuggester::suggest(legacy, safety, targetLang);

        json suggestions = json::array();
        for (const auto& s : report.suggestions)
            suggestions.push_back(suggestionToJson(s));

        return {{"filePath", report.filePath},
                {"sourceLanguage", report.sourceLanguage},
                {"targetLanguage", report.targetLanguage},
                {"quickWinCount", report.countByEffort(ModernizeEffort::QuickWin)},
                {"moderateCount", report.countByEffort(ModernizeEffort::Moderate)},
                {"deepRefactorCount", report.countByEffort(ModernizeEffort::DeepRefactor)},
                {"suggestions", suggestions}};
    }

    // createModernizationWorkflow: full pipeline → workflow
    static json handleCreateWorkflow(const json& params) {
        std::string source = params.value("source", "");
        std::string language = params.value("language", "c");
        std::string filePath = params.value("filePath", "unknown");
        std::string targetLang = params.value("targetLanguage", "");

        auto legacy = LegacyIdiomDetector::analyzeFile(source, language, filePath);
        auto safety = SafetyAuditor::audit(source, language, filePath);
        auto report = ModernizationSuggester::suggest(legacy, safety, targetLang);
        auto wf = ModernizationWorkflowGenerator::generate(source, report);

        json items = json::array();
        for (const auto& item : wf.itemsInOrder())
            items.push_back(workItemToJson(item));

        return {{"filePath", wf.filePath},
                {"sourceLanguage", wf.sourceLanguage},
                {"targetLanguage", wf.targetLanguage},
                {"itemCount", (int)wf.items.size()},
                {"deterministicCount", wf.countByRouting(WorkItemRouting::Deterministic)},
                {"llmCount", wf.countByRouting(WorkItemRouting::LLM)},
                {"humanCount", wf.countByRouting(WorkItemRouting::Human)},
                {"items", items},
                {"skeleton", {
                    {"language", wf.skeleton.language},
                    {"annotations", wf.skeleton.annotations},
                    {"source", wf.skeleton.skeletonSource}
                }}};
    }

    // Dispatch: route method name to handler
    static bool canHandle(const std::string& method) {
        return method == "analyzeLegacy" ||
               method == "getSafetyReport" ||
               method == "suggestModernization" ||
               method == "createModernizationWorkflow";
    }

    static json dispatch(const std::string& method, const json& params, const json& id) {
        json result;
        if (method == "analyzeLegacy")
            result = handleAnalyzeLegacy(params);
        else if (method == "getSafetyReport")
            result = handleGetSafetyReport(params);
        else if (method == "suggestModernization")
            result = handleSuggestModernization(params);
        else if (method == "createModernizationWorkflow")
            result = handleCreateWorkflow(params);
        else
            return {{"jsonrpc", "2.0"}, {"id", id},
                    {"error", {{"code", -32601}, {"message", "Unknown method"}}}};
        return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
    }

    // --- MCP Tool Definitions ---

    struct MCPToolDef {
        std::string name;
        std::string description;
        json inputSchema;
    };

    static std::vector<MCPToolDef> toolDefinitions() {
        json sourceSchema = {
            {"type", "object"},
            {"properties", {
                {"source", {{"type", "string"}, {"description", "Source code to analyze"}}},
                {"language", {{"type", "string"}, {"description", "Language: c, cpp, python, etc."}}},
                {"filePath", {{"type", "string"}, {"description", "File path for reporting"}}}
            }},
            {"required", json::array({"source"})}
        };

        json modernizeSchema = sourceSchema;
        modernizeSchema["properties"]["targetLanguage"] = {
            {"type", "string"},
            {"description", "Target language for cross-language modernization (optional)"}
        };

        return {
            {"whetstone_analyze_legacy",
             "Run legacy idiom detection on source code. Detects K&R declarations, "
             "goto flow, deprecated APIs (gets/sprintf/strcpy), pointer arithmetic, "
             "manual memory management. Returns legacy score (0-10) and findings.",
             sourceSchema},
            {"whetstone_get_safety_report",
             "Run structured safety audit on source code. Detects buffer overflow, "
             "use-after-free, null dereference, race conditions, integer overflow. "
             "Maps findings to CWE codes with risk levels (0-4).",
             sourceSchema},
            {"whetstone_suggest_modernization",
             "Generate modernization suggestions for legacy code. Maps each legacy "
             "pattern to a modern replacement with @Modernize annotations. Groups "
             "suggestions by effort: quick wins, moderate, deep refactors.",
             modernizeSchema},
            {"whetstone_create_modernization_workflow",
             "Full pipeline: analyze legacy code → safety audit → suggest modernization "
             "→ generate executable workflow. Returns ordered work items with routing "
             "(deterministic/LLM/human) and a modernized skeleton.",
             modernizeSchema}
        };
    }
};
