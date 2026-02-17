#pragma once

// Step 496: Self-Annotated Workflow
// Models modernization workflow for AnnotationValidator.h.

#include "Pipeline.h"

#include <string>
#include <vector>

enum class ModernizationRuleKind {
    SimpleRule,
    ComplexRule,
    NewDiagnosticCode
};

struct ModernizationRuleSpec {
    std::string ruleId;
    std::string description;
    ModernizationRuleKind kind = ModernizationRuleKind::SimpleRule;
    std::string targetDiagnosticCode; // e.g. E1491
};

struct ModernizationTask {
    std::string taskId;
    std::string ruleId;
    std::string routeTo; // deterministic | llm | human
    bool reviewRequired = false;
    std::string priority;
    std::vector<std::string> dependencies;
    std::string generatedCode; // deterministic output if executed
    bool outputValid = false;
};

struct ModernizationWorkflowReport {
    std::string targetFile;
    std::vector<ModernizationRuleSpec> rules;
    std::vector<ModernizationTask> tasks;
    int deterministicCount = 0;
    int llmCount = 0;
    int humanCount = 0;
    int deterministicValidCount = 0;
    std::vector<std::string> notes;
};

class SelfModernizationWorkflow {
public:
    static std::vector<ModernizationRuleSpec> defaultRules() {
        return {
            {"rule_owner_single_alias_guard",
             "Add explicit owner aliasing diagnostic for nested assignment chains",
             ModernizationRuleKind::SimpleRule, "E1491"},
            {"rule_deallocate_location_required",
             "Strengthen deallocate validation with location semantics",
             ModernizationRuleKind::SimpleRule, "E1492"},
            {"rule_cross_subject_consistency",
             "Validate cross-subject annotation consistency between risk/owner/lifetime",
             ModernizationRuleKind::ComplexRule, "E1493"},
            {"rule_recursive_tailcall_guard",
             "Validate recursive visitor tail-call annotations across nested traversals",
             ModernizationRuleKind::ComplexRule, "E1494"},
            {"rule_new_diagnostic_code_registry",
             "Reserve and validate new diagnostic code range for modernization rules",
             ModernizationRuleKind::NewDiagnosticCode, "E1495"}
        };
    }

    static ModernizationWorkflowReport createAndRun(const std::string& targetFile,
                                                    const std::vector<ModernizationRuleSpec>& rules =
                                                        defaultRules()) {
        ModernizationWorkflowReport out;
        out.targetFile = targetFile;
        out.rules = rules;

        for (size_t i = 0; i < rules.size(); ++i) {
            ModernizationTask t;
            t.taskId = "self-modernize-" + std::to_string(i + 1);
            t.ruleId = rules[i].ruleId;
            routeTask(rules[i], t);
            out.tasks.push_back(t);
        }

        executeDeterministic(out.tasks, rules, out);
        out.notes.push_back("Modernization workflow created for AnnotationValidator self-improvement");
        return out;
    }

private:
    static void routeTask(const ModernizationRuleSpec& rule, ModernizationTask& task) {
        if (rule.kind == ModernizationRuleKind::SimpleRule) {
            task.routeTo = "deterministic";
            task.priority = "high";
            task.reviewRequired = false;
            return;
        }
        if (rule.kind == ModernizationRuleKind::ComplexRule) {
            task.routeTo = "llm";
            task.priority = "high";
            task.reviewRequired = true;
            return;
        }
        task.routeTo = "human";
        task.priority = "critical";
        task.reviewRequired = true;
    }

    static void executeDeterministic(std::vector<ModernizationTask>& tasks,
                                     const std::vector<ModernizationRuleSpec>& rules,
                                     ModernizationWorkflowReport& out) {
        for (auto& task : tasks) {
            if (task.routeTo == "deterministic") {
                const auto* rule = findRule(rules, task.ruleId);
                task.generatedCode = generateDeterministicSnippet(*rule);
                task.outputValid = validateSnippet(task.generatedCode);
                out.deterministicCount++;
                if (task.outputValid) out.deterministicValidCount++;
            } else if (task.routeTo == "llm") {
                out.llmCount++;
            } else if (task.routeTo == "human") {
                out.humanCount++;
            }
        }
    }

    static const ModernizationRuleSpec* findRule(const std::vector<ModernizationRuleSpec>& rules,
                                                 const std::string& ruleId) {
        for (const auto& r : rules) if (r.ruleId == ruleId) return &r;
        return &rules.front();
    }

    static std::string generateDeterministicSnippet(const ModernizationRuleSpec& rule) {
        std::string fn = "validate_" + rule.ruleId;
        for (auto& c : fn) if (c == '-') c = '_';
        return
            "bool " + fn + "(const ASTNode* node, std::vector<AnnotationValidator::Diagnostic>& diags) {\n"
            "    if (!node) return true;\n"
            "    // STUB: " + rule.description + "\n"
            "    if (node->id.empty()) {\n"
            "        diags.push_back({\"warning\", \"" + rule.targetDiagnosticCode + ": missing node id\", \"\"});\n"
            "        return false;\n"
            "    }\n"
            "    return true;\n"
            "}\n";
    }

    static bool validateSnippet(const std::string& snippet) {
        Pipeline p;
        std::vector<ParseDiagnostic> diags;
        const std::string src =
            "#include <string>\n#include <vector>\n"
            "struct ASTNode { std::string id; };\n"
            "struct AnnotationValidator { struct Diagnostic { std::string severity; std::string message; std::string nodeId; }; };\n"
            + snippet;
        auto mod = p.parse(src, "cpp", diags);
        return mod != nullptr;
    }
};
