#pragma once
// Step 541: Rust/Go Constructive Edit Adapter

#include <set>
#include <string>
#include <vector>

struct RustGoConstructiveEditRequest {
    std::string language;      // rust/go
    std::string constructKind; // function/struct/impl/import/channel/borrow_region
    std::string operation;
    bool borrowOrLifetimeRisk = false;
    bool concurrencyRisk = false;
};

struct RustGoConstructiveEditResult {
    bool supported = false;
    bool allowed = false;
    std::vector<std::string> legalOperations;
    std::vector<std::string> diagnostics;
};

class RustGoConstructiveEditAdapter {
public:
    static RustGoConstructiveEditResult evaluate(const RustGoConstructiveEditRequest& request) {
        RustGoConstructiveEditResult result;
        if (request.language != "rust" && request.language != "go") {
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

        auto guard = safetyGuard(request);
        if (!guard.empty()) {
            result.diagnostics.push_back(guard);
            return result;
        }

        result.allowed = true;
        return result;
    }

private:
    static std::vector<std::string> legalOps(const std::string& kind) {
        if (kind == "function") return {"rename", "update", "extract"};
        if (kind == "struct") return {"rename", "update"};
        if (kind == "impl") return {"update", "extract"};
        if (kind == "import") return {"insert", "delete", "reorder"};
        if (kind == "channel") return {"update", "extract"};
        if (kind == "borrow_region") return {"update", "extract"};
        return {};
    }

    static std::string safetyGuard(const RustGoConstructiveEditRequest& request) {
        if (request.language == "rust" && request.borrowOrLifetimeRisk) {
            if (request.constructKind == "borrow_region" && request.operation == "extract") {
                return "rust_borrow_lifetime_extract_blocked";
            }
            if (request.constructKind == "struct" && request.operation == "rename") {
                return "rust_lifetime_sensitive_rename_blocked";
            }
        }

        if (request.language == "go" && request.concurrencyRisk) {
            if (request.constructKind == "channel" && request.operation == "extract") {
                return "go_concurrency_extract_blocked";
            }
            if (request.constructKind == "function" && request.operation == "update") {
                return "go_concurrency_update_blocked";
            }
        }
        return "";
    }

    static bool contains(const std::vector<std::string>& values,
                         const std::string& value) {
        return std::set<std::string>(values.begin(), values.end()).count(value) != 0;
    }
};
