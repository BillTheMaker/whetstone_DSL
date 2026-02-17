#pragma once
// Step 536: Argument Shape Validator

#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "TypedTaskitemContractSchema.h"

using json = nlohmann::json;

struct ArgumentShapeResult {
    bool valid = false;
    std::vector<std::string> errors;
};

class ArgumentShapeValidator {
public:
    static ArgumentShapeResult validate(const TaskitemContract& contract,
                                        const std::string& operation,
                                        const std::string& symbol,
                                        const json& args) {
        ArgumentShapeResult result;

        if (!TypedTaskitemContractSchema::opAllowed(contract, operation)) {
            result.errors.push_back("op_not_allowed:" + operation);
        }
        if (!TypedTaskitemContractSchema::symbolAllowed(contract, symbol)) {
            result.errors.push_back("symbol_not_allowed:" + symbol);
        }
        if (!args.is_object()) {
            result.errors.push_back("args_not_object");
            result.valid = false;
            return result;
        }

        if (operation == "rename") {
            requireString(args, "newName", result.errors);
            rejectSameValue(args, "newName", symbol, "rename_noop", result.errors);
            forbidKeys(args, {"replacement", "insertText", "position", "mode", "target"}, result.errors);
        } else if (operation == "update") {
            requireString(args, "replacement", result.errors);
            forbidKeys(args, {"newName", "insertText", "position", "target"}, result.errors);
        } else if (operation == "insert") {
            requireString(args, "insertText", result.errors);
            requireString(args, "position", result.errors);
            validateEnum(args, "position", {"before", "after", "inside"}, result.errors);
            forbidKeys(args, {"newName", "replacement"}, result.errors);
        } else if (operation == "extract") {
            requireString(args, "target", result.errors);
            requireString(args, "mode", result.errors);
            validateEnum(args, "mode", {"function", "constant"}, result.errors);
        } else if (operation == "delete") {
            requireBoolean(args, "confirm", result.errors);
            forbidKeys(args, {"newName", "replacement", "insertText"}, result.errors);
        } else {
            result.errors.push_back("unsupported_operation_shape:" + operation);
        }

        result.valid = result.errors.empty();
        return result;
    }

private:
    static void requireString(const json& args,
                              const std::string& key,
                              std::vector<std::string>& errors) {
        if (!args.contains(key) || !args[key].is_string() || args[key].get<std::string>().empty()) {
            errors.push_back("missing_or_invalid_string:" + key);
        }
    }

    static void requireBoolean(const json& args,
                               const std::string& key,
                               std::vector<std::string>& errors) {
        if (!args.contains(key) || !args[key].is_boolean()) {
            errors.push_back("missing_or_invalid_boolean:" + key);
        }
    }

    static void rejectSameValue(const json& args,
                                const std::string& key,
                                const std::string& value,
                                const std::string& code,
                                std::vector<std::string>& errors) {
        if (!args.contains(key) || !args[key].is_string()) return;
        if (args[key].get<std::string>() == value) {
            errors.push_back(code);
        }
    }

    static void validateEnum(const json& args,
                             const std::string& key,
                             const std::set<std::string>& allowed,
                             std::vector<std::string>& errors) {
        if (!args.contains(key) || !args[key].is_string()) return;
        if (allowed.count(args[key].get<std::string>()) == 0) {
            errors.push_back("invalid_enum:" + key);
        }
    }

    static void forbidKeys(const json& args,
                           const std::set<std::string>& forbidden,
                           std::vector<std::string>& errors) {
        for (const auto& key : forbidden) {
            if (args.contains(key)) {
                errors.push_back("forbidden_key:" + key);
            }
        }
    }
};
