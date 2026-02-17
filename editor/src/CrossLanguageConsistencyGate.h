#pragma once
// Step 542: Cross-Language Consistency Gate

#include <string>
#include <vector>

#include "CppConstructiveEditAdapter.h"
#include "PythonTypeScriptConstructiveEditAdapter.h"
#include "RustGoConstructiveEditAdapter.h"

struct CrossLanguageRequest {
    std::string language;
    std::string constructKind;
    std::string operation;
    bool moduleSideEffectRisk = false;
    bool borrowOrLifetimeRisk = false;
    bool concurrencyRisk = false;
    std::string fileType = "source";
};

struct CrossLanguageResult {
    bool supported = false;
    bool allowed = false;
    std::string standardizedFailure = "";
    std::string fallbackAction = "none";
    std::vector<std::string> diagnostics;
};

class CrossLanguageConsistencyGate {
public:
    static CrossLanguageResult evaluate(const CrossLanguageRequest& request) {
        CrossLanguageResult out;

        if (request.language == "c" || request.language == "cpp") {
            auto r = CppConstructiveEditAdapter::evaluate(
                {request.language, request.constructKind, request.fileType, request.operation});
            return normalize(r.supported, r.allowed, r.diagnostics);
        }

        if (request.language == "python" || request.language == "typescript") {
            auto r = PythonTypeScriptConstructiveEditAdapter::evaluate(
                {request.language, request.constructKind, request.operation,
                 request.moduleSideEffectRisk});
            return normalize(r.supported, r.allowed, r.diagnostics);
        }

        if (request.language == "rust" || request.language == "go") {
            auto r = RustGoConstructiveEditAdapter::evaluate(
                {request.language, request.constructKind, request.operation,
                 request.borrowOrLifetimeRisk, request.concurrencyRisk});
            return normalize(r.supported, r.allowed, r.diagnostics);
        }

        out.diagnostics = {"unsupported_language"};
        out.standardizedFailure = "unsupported_language";
        out.fallbackAction = "fallback_to_readonly";
        return out;
    }

private:
    static CrossLanguageResult normalize(bool supported,
                                         bool allowed,
                                         const std::vector<std::string>& diagnostics) {
        CrossLanguageResult out;
        out.supported = supported;
        out.allowed = allowed;
        out.diagnostics = diagnostics;

        if (supported && allowed) {
            out.standardizedFailure = "";
            out.fallbackAction = "none";
            return out;
        }

        const std::string primary = diagnostics.empty() ? "unknown_failure" : diagnostics.front();
        out.standardizedFailure = canonical(primary);

        if (out.standardizedFailure == "unsupported_language" ||
            out.standardizedFailure == "unsupported_construct") {
            out.fallbackAction = "fallback_to_readonly";
        } else if (out.standardizedFailure == "operation_not_legal_for_construct") {
            out.fallbackAction = "suggest_legal_ops";
        } else if (out.standardizedFailure == "safety_guard_blocked") {
            out.fallbackAction = "escalate_with_safety_context";
        } else {
            out.fallbackAction = "retry_with_constraints";
        }

        return out;
    }

    static std::string canonical(const std::string& diag) {
        if (diag == "unsupported_language") return "unsupported_language";
        if (diag == "unsupported_construct") return "unsupported_construct";
        if (diag == "operation_not_legal_for_construct") return "operation_not_legal_for_construct";

        if (diag.find("boundary") != std::string::npos ||
            diag.find("side_effect") != std::string::npos ||
            diag.find("lifetime") != std::string::npos ||
            diag.find("concurrency") != std::string::npos ||
            diag.find("borrow") != std::string::npos ||
            diag.find("review") != std::string::npos) {
            return "safety_guard_blocked";
        }

        return "constraint_failure";
    }
};
