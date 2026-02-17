#pragma once
// Step 539: C/C++ Constructive Edit Adapter

#include <string>
#include <vector>

#include "AdapterOperationUtil.h"

struct CppConstructiveEditRequest {
    std::string language;      // c or cpp
    std::string constructKind; // declaration/definition/include/macro/class/member
    std::string fileType;      // header/source
    std::string operation;
};

struct CppConstructiveEditResult {
    bool supported = false;
    bool allowed = false;
    std::vector<std::string> legalOperations;
    std::vector<std::string> diagnostics;
};

class CppConstructiveEditAdapter {
public:
    static CppConstructiveEditResult evaluate(const CppConstructiveEditRequest& request) {
        CppConstructiveEditResult result;
        if (request.language != "c" && request.language != "cpp") {
            result.diagnostics.push_back("unsupported_language");
            return result;
        }

        result.supported = true;
        result.legalOperations = legalOps(request.constructKind, request.fileType);
        if (result.legalOperations.empty()) {
            result.supported = false;
            result.diagnostics.push_back("unsupported_construct");
            return result;
        }

        if (!adapterHasOperation(result.legalOperations, request.operation)) {
            result.diagnostics.push_back("operation_not_legal_for_construct");
            return result;
        }

        auto boundary = boundaryGuard(request);
        if (!boundary.empty()) {
            result.diagnostics.push_back(boundary);
            return result;
        }

        result.allowed = true;
        return result;
    }

private:
    static std::vector<std::string> legalOps(const std::string& kind,
                                             const std::string& fileType) {
        if (kind == "declaration") return {"rename", "update", "delete", "extract"};
        if (kind == "definition") return {"rename", "update", "extract", "inline"};
        if (kind == "include") return {"insert", "delete", "reorder"};
        if (kind == "macro") return {"rename", "update", "delete"};
        if (kind == "class") return {"rename", "update", "extract"};
        if (kind == "member") {
            if (fileType == "header") return {"rename", "update", "extract"};
            return {"rename", "update"};
        }
        return {};
    }

    static std::string boundaryGuard(const CppConstructiveEditRequest& request) {
        // Header/source safety: definitions and include edits should stay in safe domains.
        if (request.constructKind == "definition" && request.fileType == "header" &&
            request.operation == "inline") {
            return "header_source_boundary_violation";
        }
        if (request.constructKind == "include" && request.fileType == "source" &&
            request.operation == "reorder") {
            return "source_include_reorder_blocked";
        }
        if (request.constructKind == "macro" && request.fileType == "source" &&
            request.operation == "delete") {
            return "macro_delete_requires_header_review";
        }
        return "";
    }

};
