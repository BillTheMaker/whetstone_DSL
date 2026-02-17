#pragma once

// Step 458: Transpilation RPC + MCP tool definitions for semantic transpilation,
// reporting, equivalence checks, and confidence scoring.

#include "BehavioralEquivalence.h"
#include "IntentTranslator.h"
#include "TranslationReport.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

inline json confidenceToJson(const ConfidenceScore& s) {
    return {
        {"functionName", s.functionName},
        {"sourceLanguage", s.sourceLanguage},
        {"targetLanguage", s.targetLanguage},
        {"score", s.score},
        {"reviewRequired", s.reviewRequired},
        {"annotation", s.annotation},
        {"reviewReason", s.reviewReason}
    };
}

inline json fnReportToJson(const FunctionTranslationReport& f) {
    return {
        {"functionName", f.functionName},
        {"sourceLanguage", f.sourceLanguage},
        {"targetLanguage", f.targetLanguage},
        {"sourceConstruct", f.sourceConstruct},
        {"targetConstruct", f.targetConstruct},
        {"confidence", f.confidence},
        {"idiomatic", f.idiomatic},
        {"reviewRequired", f.reviewRequired},
        {"reviewReason", f.reviewReason},
        {"safetyDelta", f.safetyDelta},
        {"rationale", f.rationale},
        {"annotations", {
            {"added", f.annotations.added},
            {"removed", f.annotations.removed},
            {"changed", f.annotations.changed}
        }}
    };
}

inline json equivalenceToJson(const BehavioralCheckResult& r) {
    json assertions = json::array();
    for (const auto& a : r.assertions) {
        assertions.push_back({
            {"functionName", a.functionName},
            {"inputSpec", a.inputSpec},
            {"expectedOutput", a.expectedOutput},
            {"sameErrors", a.sameErrors},
            {"confidenceScore", a.confidenceScore}
        });
    }
    return {
        {"functionName", r.functionName},
        {"sourceLanguage", r.sourceLanguage},
        {"targetLanguage", r.targetLanguage},
        {"assertions", assertions},
        {"allPassing", r.allPassing},
        {"annotation", r.annotation}
    };
}

class TranspilationRPCHandler {
public:
    static json handleTranspile(const json& params) {
        FunctionIntent fn{
            params.value("functionName", "function"),
            params.value("intent", ""),
            params.value("source", "")
        };
        std::string sourceLanguage = params.value("sourceLanguage", "python");
        std::string targetLanguage = params.value("targetLanguage", "rust");
        auto result = IntentTranslator::translate({fn}, sourceLanguage, targetLanguage);
        auto translation = result.getTranslation(fn.name);

        return {
            {"functionName", translation.functionName},
            {"sourceLanguage", sourceLanguage},
            {"targetLanguage", targetLanguage},
            {"sourceCode", translation.sourceCode},
            {"targetCode", translation.targetCode},
            {"confidence", translation.confidence},
            {"idiomatic", translation.isIdiomatic},
            {"needsReview", translation.needsReview},
            {"reviewReason", translation.reviewReason}
        };
    }

    static json handleGetTranslationReport(const json& params) {
        std::string sourceLanguage = params.value("sourceLanguage", "python");
        std::string targetLanguage = params.value("targetLanguage", "rust");
        json funcs = params.value("functions", json::array());
        std::vector<FunctionIntent> fns;
        for (const auto& f : funcs) {
            fns.push_back({
                f.value("name", "function"),
                f.value("intent", ""),
                f.value("source", "")
            });
        }
        auto translated = IntentTranslator::translate(fns, sourceLanguage, targetLanguage);
        auto report = TranslationReportGenerator::generate(translated);

        json functionReports = json::array();
        for (const auto& fn : report.functions)
            functionReports.push_back(fnReportToJson(fn));

        return {
            {"sourceLanguage", report.sourceLanguage},
            {"targetLanguage", report.targetLanguage},
            {"functions", functionReports},
            {"summary", {
                {"totalFunctions", report.summary.totalFunctions},
                {"idiomaticCount", report.summary.idiomaticCount},
                {"literalCount", report.summary.literalCount},
                {"flaggedForReviewCount", report.summary.flaggedForReviewCount},
                {"idiomaticPercent", report.summary.idiomaticPercent()},
                {"literalPercent", report.summary.literalPercent()},
                {"reviewPercent", report.summary.reviewPercent()}
            }}
        };
    }

    static json handleVerifyEquivalence(const json& params) {
        std::string functionName = params.value("functionName", "function");
        std::string sourceCode = params.value("sourceCode", "");
        std::string targetCode = params.value("targetCode", "");
        std::string sourceLanguage = params.value("sourceLanguage", "python");
        std::string targetLanguage = params.value("targetLanguage", "rust");

        std::vector<std::string> sampleInputs;
        if (params.contains("sampleInputs")) {
            for (const auto& s : params["sampleInputs"])
                sampleInputs.push_back(s.get<std::string>());
        }

        auto result = BehavioralChecker::check(
            functionName, sourceCode, targetCode, sourceLanguage, targetLanguage, sampleInputs);
        return equivalenceToJson(result);
    }

    static json handleGetConfidence(const json& params) {
        std::string functionName = params.value("functionName", "function");
        std::string sourceCode = params.value("sourceCode", "");
        std::string targetCode = params.value("targetCode", "");
        std::string sourceLanguage = params.value("sourceLanguage", "python");
        std::string targetLanguage = params.value("targetLanguage", "rust");
        bool hasIntent = params.value("hasIntent", false);

        auto score = TranspilationScorer::score(
            functionName, sourceCode, targetCode, sourceLanguage, targetLanguage, hasIntent);
        return confidenceToJson(score);
    }

    static bool canHandle(const std::string& method) {
        return method == "transpile" ||
               method == "getTranslationReport" ||
               method == "verifyEquivalence" ||
               method == "getConfidence";
    }

    static json dispatch(const std::string& method, const json& params, const json& id) {
        json result;
        if (method == "transpile")
            result = handleTranspile(params);
        else if (method == "getTranslationReport")
            result = handleGetTranslationReport(params);
        else if (method == "verifyEquivalence")
            result = handleVerifyEquivalence(params);
        else if (method == "getConfidence")
            result = handleGetConfidence(params);
        else
            return {{"jsonrpc", "2.0"}, {"id", id},
                    {"error", {{"code", -32601}, {"message", "Unknown method"}}}};
        return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
    }

    struct MCPToolDef {
        std::string name;
        std::string description;
        json inputSchema;
    };

    static std::vector<MCPToolDef> toolDefinitions() {
        json transpileSchema = {
            {"type", "object"},
            {"properties", {
                {"functionName", {{"type", "string"}}},
                {"intent", {{"type", "string"}}},
                {"source", {{"type", "string"}}},
                {"sourceLanguage", {{"type", "string"}}},
                {"targetLanguage", {{"type", "string"}}}
            }},
            {"required", json::array({"functionName", "source"})}
        };

        json reportSchema = {
            {"type", "object"},
            {"properties", {
                {"functions", {{"type", "array"}}},
                {"sourceLanguage", {{"type", "string"}}},
                {"targetLanguage", {{"type", "string"}}}
            }},
            {"required", json::array({"functions"})}
        };

        json eqSchema = {
            {"type", "object"},
            {"properties", {
                {"functionName", {{"type", "string"}}},
                {"sourceCode", {{"type", "string"}}},
                {"targetCode", {{"type", "string"}}},
                {"sourceLanguage", {{"type", "string"}}},
                {"targetLanguage", {{"type", "string"}}},
                {"sampleInputs", {{"type", "array"}}}
            }},
            {"required", json::array({"functionName", "sourceCode", "targetCode"})}
        };

        json confidenceSchema = {
            {"type", "object"},
            {"properties", {
                {"functionName", {{"type", "string"}}},
                {"sourceCode", {{"type", "string"}}},
                {"targetCode", {{"type", "string"}}},
                {"sourceLanguage", {{"type", "string"}}},
                {"targetLanguage", {{"type", "string"}}},
                {"hasIntent", {{"type", "boolean"}}}
            }},
            {"required", json::array({"functionName", "sourceCode", "targetCode"})}
        };

        return {
            {"whetstone_transpile",
             "Transpile a function using semantic intent-aware translation.",
             transpileSchema},
            {"whetstone_get_translation_report",
             "Generate per-function and project-level translation report.",
             reportSchema},
            {"whetstone_verify_equivalence",
             "Generate behavioral equivalence assertions for source vs target.",
             eqSchema},
            {"whetstone_get_confidence",
             "Get confidence score and review requirement for a translation.",
             confidenceSchema}
        };
    }
};
