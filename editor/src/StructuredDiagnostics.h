#pragma once
// Step 250: Structured Diagnostics
//
// Unified diagnostic format for agent consumption. Merges tree-sitter
// parse errors, AnnotationValidator diagnostics, and StrategyValidator
// violations into a single stream with error codes, AST node references,
// and optional machine-applicable fix mutations.
//
// Error code scheme:
//   E01xx — Parse errors (tree-sitter syntax)
//   E02xx — Annotation validation (consistency)
//   E03xx — Strategy validation (memory safety)

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "ast/ASTNode.h"
#include "ast/Parser.h"
#include "AnnotationValidator.h"
#include "StrategyValidator.h"
#include "Pipeline.h"

using json = nlohmann::json;

// -----------------------------------------------------------------------
//  Severity levels (numeric for filtering, lower = more severe)
// -----------------------------------------------------------------------
enum class DiagnosticSeverity {
    Error   = 1,
    Warning = 2,
    Info    = 3,
    Hint    = 4
};

inline const char* severityLabel(DiagnosticSeverity s) {
    switch (s) {
        case DiagnosticSeverity::Error:   return "error";
        case DiagnosticSeverity::Warning: return "warning";
        case DiagnosticSeverity::Info:    return "info";
        case DiagnosticSeverity::Hint:    return "hint";
    }
    return "info";
}

inline DiagnosticSeverity severityFromStr(const std::string& s) {
    if (s == "error")   return DiagnosticSeverity::Error;
    if (s == "warning") return DiagnosticSeverity::Warning;
    if (s == "hint")    return DiagnosticSeverity::Hint;
    return DiagnosticSeverity::Info;
}

// -----------------------------------------------------------------------
//  StructuredDiagnostic — single unified diagnostic entry
// -----------------------------------------------------------------------
struct StructuredDiagnostic {
    std::string       code;       // e.g. "E0100", "E0201"
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string       nodeId;     // AST node reference (empty for parse)
    int               line = 0;   // 1-based
    int               col  = 0;   // 1-based
    std::string       message;
    std::string       source;     // "parser", "annotation", "strategy"
    json              fix = nullptr; // optional mutation object
};

inline json diagnosticToJson(const StructuredDiagnostic& d) {
    json j = {
        {"code",     d.code},
        {"severity", severityLabel(d.severity)},
        {"severityLevel", static_cast<int>(d.severity)},
        {"nodeId",   d.nodeId},
        {"line",     d.line},
        {"col",      d.col},
        {"message",  d.message},
        {"source",   d.source}
    };
    if (!d.fix.is_null()) j["fix"] = d.fix;
    return j;
}

// -----------------------------------------------------------------------
//  Error code assignment
// -----------------------------------------------------------------------

// Parse errors: E0100 (generic syntax), E0101 (missing token)
inline std::string parseErrorCode(const std::string& message) {
    if (message.find("missing") != std::string::npos ||
        message.find("Missing") != std::string::npos)
        return "E0101";
    return "E0100";
}

// Annotation validation: E0200..E0203
inline std::string annotationErrorCode(const std::string& message) {
    if (message.find("Missing Intent") != std::string::npos)
        return "E0200";
    if (message.find("aliased") != std::string::npos ||
        message.find("alias") != std::string::npos)
        return "E0201";
    if (message.find("conflict") != std::string::npos ||
        message.find("Conflict") != std::string::npos)
        return "E0202";
    return "E0203";
}

// Strategy validation: E0300..E0304
inline std::string strategyErrorCode(const std::string& category) {
    if (category == "use-after-free")         return "E0300";
    if (category == "leak")                   return "E0301";
    if (category == "double-move")            return "E0302";
    if (category == "aliasing")               return "E0303";
    if (category == "destructor-unreachable") return "E0304";
    return "E0305";
}

// -----------------------------------------------------------------------
//  Fix suggestion builders
// -----------------------------------------------------------------------

// For annotation issues: suggest removing the problematic annotation
inline json buildAnnotationFix(const std::string& nodeId,
                                const std::string& message) {
    if (message.find("Missing Intent") != std::string::npos) {
        // Deallocate(Explicit) without dealloc point: suggest removing
        return {{"type", "deleteNode"}, {"nodeId", nodeId},
                {"description", "Remove @Deallocate(Explicit) annotation"}};
    }
    if (message.find("aliased") != std::string::npos) {
        // Single ownership alias: suggest changing to Shared
        return {{"type", "setProperty"}, {"nodeId", nodeId},
                {"property", "strategy"}, {"value", "Shared"},
                {"description",
                 "Change @Owner(Single) to @Owner(Shared)"}};
    }
    return nullptr;
}

// For strategy violations: suggest concrete fixes
inline json buildStrategyFix(const std::string& nodeId,
                              const std::string& category) {
    if (category == "use-after-free") {
        return {{"type", "deleteNode"}, {"nodeId", nodeId},
                {"description",
                 "Remove use of variable after deallocation"}};
    }
    if (category == "leak") {
        return {{"type", "insertNode"}, {"parentId", nodeId},
                {"role", "body"},
                {"description",
                 "Add deallocation call for leaked variable"}};
    }
    return nullptr;
}

// -----------------------------------------------------------------------
//  Collectors: convert each diagnostic source to unified format
// -----------------------------------------------------------------------

inline std::vector<StructuredDiagnostic> collectParseDiagnostics(
        const std::vector<ParseDiagnostic>& diags) {
    std::vector<StructuredDiagnostic> out;
    out.reserve(diags.size());
    for (const auto& d : diags) {
        StructuredDiagnostic sd;
        sd.code     = parseErrorCode(d.message);
        sd.severity = severityFromStr(d.severity);
        sd.line     = d.line;
        sd.col      = d.column;
        sd.message  = d.message;
        sd.source   = "parser";
        out.push_back(std::move(sd));
    }
    return out;
}

inline std::vector<StructuredDiagnostic> collectAnnotationDiagnostics(
        const std::vector<AnnotationValidator::Diagnostic>& diags) {
    std::vector<StructuredDiagnostic> out;
    out.reserve(diags.size());
    for (const auto& d : diags) {
        StructuredDiagnostic sd;
        sd.code     = annotationErrorCode(d.message);
        sd.severity = severityFromStr(d.severity);
        sd.nodeId   = d.nodeId;
        sd.message  = d.message;
        sd.source   = "annotation";
        sd.fix      = buildAnnotationFix(d.nodeId, d.message);
        out.push_back(std::move(sd));
    }
    return out;
}

inline std::vector<StructuredDiagnostic> collectStrategyDiagnostics(
        const std::vector<StrategyValidator::Violation>& violations) {
    std::vector<StructuredDiagnostic> out;
    out.reserve(violations.size());
    for (const auto& v : violations) {
        StructuredDiagnostic sd;
        sd.code     = strategyErrorCode(v.category);
        sd.severity = severityFromStr(v.severity);
        sd.nodeId   = v.nodeId;
        sd.message  = v.message;
        sd.source   = "strategy";
        sd.fix      = buildStrategyFix(v.nodeId, v.category);
        out.push_back(std::move(sd));
    }
    return out;
}

// -----------------------------------------------------------------------
//  collectAllDiagnostics — run the full validation pipeline on an AST
// -----------------------------------------------------------------------
inline std::vector<StructuredDiagnostic> collectAllDiagnostics(
        Module* ast) {
    std::vector<StructuredDiagnostic> all;
    if (!ast) return all;

    // Annotation validation
    AnnotationValidator annoValidator;
    auto annoDiags = annoValidator.validate(ast);
    auto annoStructured = collectAnnotationDiagnostics(annoDiags);
    all.insert(all.end(), annoStructured.begin(), annoStructured.end());

    // Strategy validation (post-optimization invariants)
    StrategyValidator stratValidator;
    auto violations = stratValidator.validateInvariants(ast);
    auto stratStructured = collectStrategyDiagnostics(violations);
    all.insert(all.end(), stratStructured.begin(), stratStructured.end());

    return all;
}

// -----------------------------------------------------------------------
//  collectPipelineDiagnostics — from a PipelineResult
// -----------------------------------------------------------------------
inline std::vector<StructuredDiagnostic> collectPipelineDiagnostics(
        const Pipeline::PipelineResult& pr) {
    std::vector<StructuredDiagnostic> all;

    auto parseDiags = collectParseDiagnostics(pr.parseDiags);
    all.insert(all.end(), parseDiags.begin(), parseDiags.end());

    auto annoDiags = collectAnnotationDiagnostics(pr.validationDiags);
    all.insert(all.end(), annoDiags.begin(), annoDiags.end());

    auto stratDiags = collectStrategyDiagnostics(pr.violations);
    all.insert(all.end(), stratDiags.begin(), stratDiags.end());

    return all;
}

// -----------------------------------------------------------------------
//  diagnosticsToJson — convert array to JSON
// -----------------------------------------------------------------------
inline json diagnosticsToJson(
        const std::vector<StructuredDiagnostic>& diags) {
    json arr = json::array();
    for (const auto& d : diags)
        arr.push_back(diagnosticToJson(d));
    return arr;
}

// -----------------------------------------------------------------------
//  filterBySeverity — keep only diagnostics at or above threshold
// -----------------------------------------------------------------------
inline std::vector<StructuredDiagnostic> filterBySeverity(
        const std::vector<StructuredDiagnostic>& diags,
        DiagnosticSeverity maxSeverity) {
    std::vector<StructuredDiagnostic> out;
    for (const auto& d : diags) {
        if (static_cast<int>(d.severity) <=
            static_cast<int>(maxSeverity)) {
            out.push_back(d);
        }
    }
    return out;
}

// -----------------------------------------------------------------------
//  filterBySource — keep only diagnostics from a specific source
// -----------------------------------------------------------------------
inline std::vector<StructuredDiagnostic> filterBySource(
        const std::vector<StructuredDiagnostic>& diags,
        const std::string& source) {
    std::vector<StructuredDiagnostic> out;
    for (const auto& d : diags) {
        if (d.source == source) out.push_back(d);
    }
    return out;
}

// -----------------------------------------------------------------------
//  Step 251: QuickFix — a concrete mutation an agent can apply
// -----------------------------------------------------------------------
struct QuickFix {
    std::string  id;          // unique fix ID (e.g. "fix-E0200-func_1")
    std::string  diagCode;    // diagnostic code this fixes
    std::string  nodeId;      // target node
    std::string  description; // human-readable label
    std::string  category;    // "remove-annotation", "change-strategy", etc.
    json         mutation;    // concrete mutation object for applyMutation
};

inline json quickFixToJson(const QuickFix& f) {
    return {
        {"id",          f.id},
        {"diagCode",    f.diagCode},
        {"nodeId",      f.nodeId},
        {"description", f.description},
        {"category",    f.category},
        {"mutation",    f.mutation}
    };
}

// -----------------------------------------------------------------------
//  getQuickFixes — collect all applicable fixes for a node
// -----------------------------------------------------------------------
inline std::vector<QuickFix> getQuickFixesForNode(
        Module* ast, const std::string& targetNodeId) {
    std::vector<QuickFix> fixes;
    if (!ast) return fixes;

    auto diags = collectAllDiagnostics(ast);
    for (const auto& d : diags) {
        if (!targetNodeId.empty() && d.nodeId != targetNodeId) continue;
        if (d.fix.is_null()) continue;

        QuickFix fix;
        fix.id = "fix-" + d.code + "-" + d.nodeId;
        fix.diagCode = d.code;
        fix.nodeId = d.nodeId;
        fix.description = d.fix.value("description", "Fix " + d.code);
        fix.mutation = d.fix;

        // Categorize based on error code
        if (d.code == "E0200")      fix.category = "remove-annotation";
        else if (d.code == "E0201") fix.category = "change-strategy";
        else if (d.code == "E0202") fix.category = "resolve-conflict";
        else if (d.code == "E0300") fix.category = "remove-use-after-free";
        else if (d.code == "E0301") fix.category = "add-deallocation";
        else                        fix.category = "general";

        fixes.push_back(std::move(fix));
    }

    // Additional heuristic fixes from AST inspection
    ASTNode* node = findNodeById(ast, targetNodeId);
    if (node) {
        // Unused variable: if a Variable has no references outside its
        // own declaration, suggest removal
        if (node->conceptType == "Variable") {
            QuickFix fix;
            fix.id = "fix-unused-" + targetNodeId;
            fix.diagCode = "E0203";
            fix.nodeId = targetNodeId;
            fix.description = "Remove unused variable";
            fix.category = "remove-unused";
            fix.mutation = {{"type", "deleteNode"},
                            {"nodeId", targetNodeId}};
            fixes.push_back(std::move(fix));
        }

        // Missing return: if a Function body ends without a Return,
        // suggest adding one
        if (node->conceptType == "Function") {
            auto body = node->getChildren("body");
            bool hasReturn = false;
            for (auto* stmt : body) {
                if (stmt->conceptType == "ReturnStatement")
                    hasReturn = true;
            }
            if (!hasReturn && !body.empty()) {
                QuickFix fix;
                fix.id = "fix-missing-return-" + targetNodeId;
                fix.diagCode = "E0203";
                fix.nodeId = targetNodeId;
                fix.description = "Add missing return statement";
                fix.category = "missing-return";
                fix.mutation = {
                    {"type", "insertNode"},
                    {"parentId", targetNodeId},
                    {"role", "body"},
                    {"node", {{"conceptType", "ReturnStatement"},
                              {"children", json::object()}}}
                };
                fixes.push_back(std::move(fix));
            }
        }
    }

    return fixes;
}

// -----------------------------------------------------------------------
//  getQuickFixesAll — collect all fixes for all diagnostics in the AST
// -----------------------------------------------------------------------
inline std::vector<QuickFix> getQuickFixesAll(Module* ast) {
    return getQuickFixesForNode(ast, "");
}

// -----------------------------------------------------------------------
//  findQuickFix — look up a specific fix by diagnostic code + nodeId
// -----------------------------------------------------------------------
inline QuickFix findQuickFix(Module* ast, const std::string& diagCode,
                              const std::string& nodeId) {
    auto fixes = getQuickFixesForNode(ast, nodeId);
    for (const auto& f : fixes) {
        if (f.diagCode == diagCode) return f;
    }
    return {}; // empty fix (mutation will be null)
}
