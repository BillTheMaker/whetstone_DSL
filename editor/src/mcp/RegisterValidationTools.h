// Step 682: MCP wiring for taskitem validation
//
// Registers:
//   whetstone_validate_taskitem
//
// This file is #included inside the MCPServer class body.

#include "../TaskitemQualityAuditor.h"

#include <filesystem>

    void registerValidationTools() {
        tools_.push_back({"whetstone_validate_taskitem",
            "Validate taskitem self-containment quality and resolve prerequisite ops "
            "against a workspace root.",
            {{"type", "object"}, {"properties", {
                {"taskitems", {{"type", "array"},
                    {"description", "One or more taskitems to validate."},
                    {"items", {{"type", "object"}, {"properties", {
                        {"task_id", {{"type", "string"}}},
                        {"title", {{"type", "string"}}},
                        {"prerequisite_ops", {{"type", "array"}, {"items", {{"type", "string"}}}}},
                        {"reasons", {{"type", "array"}, {"items", {{"type", "string"}}}}},
                        {"confidence", {{"type", "integer"}}},
                        {"dependency_task_ids", {{"type", "array"}, {"items", {{"type", "string"}}}}}
                    }}, {"required", json::array({"task_id"})}}}}},
                {"workspace", {{"type", "string"},
                    {"description", "Workspace root path for file resolution."}}}
            }}, {"required", json::array({"taskitems"})}}
        });
        toolHandlers_["whetstone_validate_taskitem"] =
            [this](const json& args) {
                return runValidateTaskitem(args);
            };
    }

    json runValidateTaskitem(const json& args) {
        if (!args.contains("taskitems") || !args["taskitems"].is_array()) {
            return {{"success", false}, {"error", "taskitems_missing_or_invalid"}};
        }

        std::vector<TaskitemInput> items;
        for (const auto& itemJson : args["taskitems"]) {
            if (!itemJson.is_object() || !itemJson.contains("task_id") || !itemJson["task_id"].is_string()) {
                return {{"success", false}, {"error", "task_id_missing"}};
            }

            TaskitemInput item;
            item.taskId = itemJson["task_id"].get<std::string>();
            item.title = itemJson.value("title", "");
            item.confidence = itemJson.value("confidence", 0);

            if (itemJson.contains("prerequisite_ops") && itemJson["prerequisite_ops"].is_array()) {
                for (const auto& v : itemJson["prerequisite_ops"]) {
                    if (v.is_string()) item.prerequisiteOps.push_back(v.get<std::string>());
                }
            }
            if (itemJson.contains("reasons") && itemJson["reasons"].is_array()) {
                for (const auto& v : itemJson["reasons"]) {
                    if (v.is_string()) item.reasons.push_back(v.get<std::string>());
                }
            }
            if (itemJson.contains("dependency_task_ids") && itemJson["dependency_task_ids"].is_array()) {
                for (const auto& v : itemJson["dependency_task_ids"]) {
                    if (v.is_string()) item.dependencyTaskIds.push_back(v.get<std::string>());
                }
            }

            items.push_back(std::move(item));
        }

        const std::string workspace = args.value("workspace", std::filesystem::current_path().generic_string());
        auto report = TaskitemQualityAuditor::audit(items, workspace);

        json results = json::array();
        for (const auto& r : report.results) {
            results.push_back({
                {"task_id", r.taskId},
                {"score", r.score},
                {"self_contained", r.selfContained},
                {"issues", r.issues}
            });
        }

        return {
            {"success", true},
            {"report", {
                {"total_taskitems", report.totalTaskitems},
                {"self_contained_count", report.selfContainedCount},
                {"warning_count", report.warningCount},
                {"failing_count", report.failingCount},
                {"average_score", report.averageScore},
                {"results", results},
                {"top_issues", report.topIssues}
            }}
        };
    }

