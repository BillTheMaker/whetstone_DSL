#pragma once

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct MCPWorkflowPromptTemplate {
    std::string name;
    std::string description;
    std::string userPrompt;
    std::vector<std::string> expectedTools;
};

class MCPWorkflowPrompts {
public:
    static std::vector<MCPWorkflowPromptTemplate> templates() {
        return {
            {
                "architect_project",
                "Analyze codebase and create annotated project skeleton.",
                "Protocol:\n"
                "1. Run whetstone_infer_annotations over key modules.\n"
                "2. Create structural scaffold with whetstone_create_skeleton.\n"
                "3. Build initial task plan using whetstone_create_workflow.\n"
                "4. Explain architecture invariants and routing assumptions.\n"
                "Goal: Analyze {{project_name}} and produce an annotated skeleton.",
                {"whetstone_infer_annotations", "whetstone_create_skeleton", "whetstone_create_workflow"}
            },
            {
                "modernize_module",
                "Modernize legacy module with workflow-guided safety.",
                "Protocol:\n"
                "1. Infer annotations and constraints for {{module_name}} with whetstone_infer_annotations.\n"
                "2. Run automatic safe tasks via whetstone_orchestrate_run_deterministic.\n"
                "3. Fetch unresolved items with whetstone_get_blockers.\n"
                "4. Submit agent changes with whetstone_submit_result.\n"
                "Goal: Modernize module while preserving behavior and auditability.",
                {"whetstone_infer_annotations", "whetstone_orchestrate_run_deterministic",
                 "whetstone_get_blockers", "whetstone_submit_result"}
            },
            {
                "port_to_language",
                "Port module between languages through workflow routing.",
                "Protocol:\n"
                "1. Read source semantics and infer annotations with whetstone_infer_annotations.\n"
                "2. Project using whetstone_project_language from {{source_language}} to {{target_language}}.\n"
                "3. Plan remaining edits with whetstone_create_workflow.\n"
                "4. Execute deterministic tasks and report blockers with whetstone_get_blockers.\n"
                "Goal: Port code to target language with semantic parity.",
                {"whetstone_infer_annotations", "whetstone_project_language",
                 "whetstone_create_workflow", "whetstone_get_blockers"}
            },
            {
                "add_feature",
                "Add feature through orchestrated skeleton-first workflow.",
                "Protocol:\n"
                "1. Add feature skeleton nodes with whetstone_create_skeleton.\n"
                "2. Create execution plan via whetstone_create_workflow.\n"
                "3. Execute deterministic tasks with whetstone_orchestrate_run_deterministic.\n"
                "4. Track progress using whetstone_get_progress.\n"
                "Goal: Implement {{feature_name}} with explicit review checkpoints.",
                {"whetstone_create_skeleton", "whetstone_create_workflow",
                 "whetstone_orchestrate_run_deterministic", "whetstone_get_progress"}
            },
            {
                "review_pending",
                "Review and approve/reject pending workflow items.",
                "Protocol:\n"
                "1. Load pending review items using whetstone_get_review_queue.\n"
                "2. For each item, open full details with whetstone_get_review_context.\n"
                "3. Approve with whetstone_approve_item or reject with whetstone_reject_item.\n"
                "4. Summarize accepted/rejected decisions and rationale.\n"
                "Goal: Clear review queue with traceable decisions.",
                {"whetstone_get_review_queue", "whetstone_get_review_context",
                 "whetstone_approve_item", "whetstone_reject_item"}
            }
        };
    }

    static bool hasTemplate(const std::string& name) {
        for (const auto& t : templates()) if (t.name == name) return true;
        return false;
    }

    static MCPWorkflowPromptTemplate getTemplate(const std::string& name) {
        for (const auto& t : templates()) if (t.name == name) return t;
        return {};
    }

    static std::string renderPrompt(const std::string& name,
                                    const std::map<std::string, std::string>& params) {
        auto t = getTemplate(name);
        std::string out = t.userPrompt;
        for (const auto& [k, v] : params) {
            replaceAll(out, "{{" + k + "}}", v);
        }
        return out;
    }

    static bool validate(const MCPWorkflowPromptTemplate& t, std::string& error) {
        if (t.name.empty()) { error = "template name is empty"; return false; }
        if (t.userPrompt.find("Protocol:") == std::string::npos) {
            error = "missing protocol section";
            return false;
        }
        if (t.expectedTools.empty()) {
            error = "expected tools list is empty";
            return false;
        }
        for (const auto& tool : t.expectedTools) {
            if (t.userPrompt.find(tool) == std::string::npos) {
                error = "prompt does not reference required tool: " + tool;
                return false;
            }
        }
        return true;
    }

    static json manifestJson() {
        json arr = json::array();
        for (const auto& t : templates()) {
            arr.push_back({
                {"name", t.name},
                {"description", t.description},
                {"expectedTools", t.expectedTools}
            });
        }
        return {{"templates", arr}};
    }

    static bool writeTemplateFiles(const std::string& outputDir) {
        std::filesystem::create_directories(outputDir);
        for (const auto& t : templates()) {
            std::ofstream out(std::filesystem::path(outputDir) / (t.name + ".prompt"));
            if (!out.is_open()) return false;
            out << "# " << t.name << "\n\n";
            out << "description:\n" << t.description << "\n\n";
            out << "user:\n" << t.userPrompt << "\n\n";
            out << "expected_tools:\n";
            for (const auto& tool : t.expectedTools) out << "- " << tool << "\n";
        }
        return true;
    }

private:
    static void replaceAll(std::string& text, const std::string& from, const std::string& to) {
        if (from.empty()) return;
        size_t pos = 0;
        while ((pos = text.find(from, pos)) != std::string::npos) {
            text.replace(pos, from.size(), to);
            pos += to.size();
        }
    }
};
