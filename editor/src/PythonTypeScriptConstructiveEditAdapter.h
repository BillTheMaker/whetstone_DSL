#pragma once
// Step 540: Python/TypeScript Constructive Edit Adapter

#include <set>
#include <string>
#include <vector>

struct PyTsConstructiveEditRequest {
    std::string language;      // python/typescript
    std::string constructKind; // function/class/import/signature/module_stmt
    std::string operation;
    bool moduleSideEffectRisk = false;
};

struct PyTsConstructiveEditResult {
    bool supported = false;
    bool allowed = false;
    std::vector<std::string> legalOperations;
    std::vector<std::string> diagnostics;
};

class PythonTypeScriptConstructiveEditAdapter {
public:
    static PyTsConstructiveEditResult evaluate(const PyTsConstructiveEditRequest& request) {
        PyTsConstructiveEditResult result;
        if (request.language != "python" && request.language != "typescript") {
            result.diagnostics.push_back("unsupported_language");
            return result;
        }

        result.supported = true;
        result.legalOperations = legalOps(request.constructKind);
        if (result.legalOperations.empty()) {
            result.supported = false;
            result.diagnostics.push_back("unsupported_construct");
            return result;
        }

        if (!contains(result.legalOperations, request.operation)) {
            result.diagnostics.push_back("operation_not_legal_for_construct");
            return result;
        }

        auto sideEffect = sideEffectGuard(request);
        if (!sideEffect.empty()) {
            result.diagnostics.push_back(sideEffect);
            return result;
        }

        result.allowed = true;
        return result;
    }

private:
    static std::vector<std::string> legalOps(const std::string& kind) {
        if (kind == "function") return {"rename", "update", "extract", "inline"};
        if (kind == "class") return {"rename", "update", "extract"};
        if (kind == "import") return {"insert", "delete", "reorder"};
        if (kind == "signature") return {"update", "extract"};
        if (kind == "module_stmt") return {"update", "delete"};
        return {};
    }

    static std::string sideEffectGuard(const PyTsConstructiveEditRequest& request) {
        // Module-level side effects: block destructive/reorder ops when flagged risky.
        if (!request.moduleSideEffectRisk) return "";

        if (request.constructKind == "import" && request.operation == "reorder") {
            return "module_side_effect_reorder_blocked";
        }
        if (request.constructKind == "module_stmt" && request.operation == "delete") {
            return "module_side_effect_delete_blocked";
        }
        if (request.constructKind == "function" && request.operation == "inline") {
            return "module_side_effect_inline_blocked";
        }
        return "";
    }

    static bool contains(const std::vector<std::string>& values,
                         const std::string& value) {
        return std::set<std::string>(values.begin(), values.end()).count(value) != 0;
    }
};
