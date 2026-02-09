#pragma once
// Step 94: Whetstone diagnostics aggregation
//
// Converts annotation/strategy diagnostics into editor-friendly diagnostics.

#include <string>
#include <vector>
#include "AnnotationValidator.h"
#include "StrategyValidator.h"

struct EditorDiagnostic {
    std::string uri;
    int line = 0;       // zero-based
    int character = 0;  // zero-based
    int severity = 0;   // 1=error, 2=warning, 3=info
    std::string message;
    std::string source;
};

inline int severityFromString(const std::string& s) {
    if (s == "error") return 1;
    if (s == "warning") return 2;
    return 3;
}

inline std::vector<EditorDiagnostic> collectWhetstoneDiagnostics(
    const std::vector<AnnotationValidator::Diagnostic>& annos,
    const std::vector<StrategyValidator::Violation>& violations,
    const std::string& uri) {
    std::vector<EditorDiagnostic> out;
    out.reserve(annos.size() + violations.size());

    for (const auto& d : annos) {
        EditorDiagnostic ed;
        ed.uri = uri;
        ed.severity = severityFromString(d.severity);
        ed.message = "[Whetstone] " + d.message;
        ed.source = "AnnotationValidator";
        out.push_back(std::move(ed));
    }

    for (const auto& v : violations) {
        EditorDiagnostic ed;
        ed.uri = uri;
        ed.severity = severityFromString(v.severity);
        ed.message = "[Whetstone] " + v.message;
        ed.source = "StrategyValidator";
        out.push_back(std::move(ed));
    }

    return out;
}
