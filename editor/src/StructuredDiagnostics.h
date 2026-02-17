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
//   E04xx — Cross-file diagnostics (imports, dependencies)

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <nlohmann/json.hpp>
#include "ast/ASTNode.h"
#include "ast/Parser.h"
#include "ast/Import.h"
#include "AnnotationValidator.h"
#include "AnnotationValidatorExtended.h"
#include "AnnotationConflictExtended.h"
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

// Annotation validation: E0200..E0203, E0600+ (extended)
inline std::string annotationErrorCode(const std::string& message) {
    // Check for embedded [Exxxx] codes first (extended validator)
    auto pos = message.find("[E");
    if (pos != std::string::npos && pos + 6 <= message.size()) {
        return message.substr(pos + 1, 5);  // extract "E0600" from "[E0600]"
    }
    // Original memory annotation codes
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

#include "diagnostics/StructuredDiagnosticsPart1.h"
#include "diagnostics/StructuredDiagnosticsPart2.h"
#include "diagnostics/StructuredDiagnosticsPart3.h"
