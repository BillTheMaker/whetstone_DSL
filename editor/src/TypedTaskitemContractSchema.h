#pragma once
// Step 524: Typed Taskitem Contract Schema

#include <string>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct TaskitemContract {
    std::string id;
    std::string nodeId;
    std::string language;
    std::vector<std::string> allowedTargets;
    std::vector<std::string> allowedOps;
    std::vector<std::string> allowedSymbols;
    std::vector<std::string> forbiddenSymbols;
    std::vector<std::string> expectedDiagnosticsAdd;
    std::vector<std::string> expectedDiagnosticsRemove;
};

struct ContractValidationResult {
    bool valid = false;
    std::vector<std::string> errors;
    TaskitemContract contract;
};

class TypedTaskitemContractSchema {
public:
    static ContractValidationResult parseAndValidate(const json& j) {
        ContractValidationResult r;

        requireString(j, "id", r.errors);
        requireString(j, "nodeId", r.errors);
        requireString(j, "language", r.errors);
        requireStringArray(j, "allowedTargets", r.errors);
        requireStringArray(j, "allowedOps", r.errors);
        requireStringArray(j, "allowedSymbols", r.errors);
        requireStringArray(j, "forbiddenSymbols", r.errors);
        requireStringArray(j, "expectedDiagnosticsAdd", r.errors);
        requireStringArray(j, "expectedDiagnosticsRemove", r.errors);

        if (r.errors.empty()) {
            r.contract.id = j["id"].get<std::string>();
            r.contract.nodeId = j["nodeId"].get<std::string>();
            r.contract.language = j["language"].get<std::string>();
            r.contract.allowedTargets = j["allowedTargets"].get<std::vector<std::string>>();
            r.contract.allowedOps = j["allowedOps"].get<std::vector<std::string>>();
            r.contract.allowedSymbols = j["allowedSymbols"].get<std::vector<std::string>>();
            r.contract.forbiddenSymbols = j["forbiddenSymbols"].get<std::vector<std::string>>();
            r.contract.expectedDiagnosticsAdd = j["expectedDiagnosticsAdd"].get<std::vector<std::string>>();
            r.contract.expectedDiagnosticsRemove = j["expectedDiagnosticsRemove"].get<std::vector<std::string>>();

            validateSemanticRules(r.contract, r.errors);
        }

        r.valid = r.errors.empty();
        return r;
    }

    static bool opAllowed(const TaskitemContract& c, const std::string& op) {
        for (const auto& a : c.allowedOps) if (a == op) return true;
        return false;
    }

    static bool symbolAllowed(const TaskitemContract& c, const std::string& sym) {
        for (const auto& f : c.forbiddenSymbols) if (f == sym) return false;
        for (const auto& a : c.allowedSymbols) if (a == sym) return true;
        return false;
    }

private:
    static void requireString(const json& j, const std::string& key,
                              std::vector<std::string>& errors) {
        if (!j.contains(key) || !j[key].is_string() || j[key].get<std::string>().empty()) {
            errors.push_back("missing-or-invalid:" + key);
        }
    }

    static void requireStringArray(const json& j, const std::string& key,
                                   std::vector<std::string>& errors) {
        if (!j.contains(key) || !j[key].is_array()) {
            errors.push_back("missing-or-invalid:" + key);
            return;
        }
        for (const auto& v : j[key]) {
            if (!v.is_string() || v.get<std::string>().empty()) {
                errors.push_back("invalid-array-item:" + key);
                return;
            }
        }
    }

    static void validateSemanticRules(const TaskitemContract& c,
                                      std::vector<std::string>& errors) {
        if (c.allowedTargets.empty()) errors.push_back("under-specified:allowedTargets");
        if (c.allowedOps.empty()) errors.push_back("under-specified:allowedOps");

        std::set<std::string> ops(c.allowedOps.begin(), c.allowedOps.end());
        if (ops.size() != c.allowedOps.size()) errors.push_back("duplicate:allowedOps");

        std::set<std::string> allowed(c.allowedSymbols.begin(), c.allowedSymbols.end());
        for (const auto& f : c.forbiddenSymbols) {
            if (allowed.count(f) > 0) {
                errors.push_back("symbol-conflict:" + f);
            }
        }

        if (c.expectedDiagnosticsAdd.empty() && c.expectedDiagnosticsRemove.empty()) {
            errors.push_back("under-specified:expectedDiagnosticsDelta");
        }
    }
};
