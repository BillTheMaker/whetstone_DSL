#pragma once

// Step 499: Scenario — Greenfield Project
// End-to-end scenario runner for "URL shortener with user accounts".

#include "ArchitectModuleDecomposer.h"
#include "ArchitectProblemParser.h"
#include "ArchitectScaffoldGenerator.h"
#include "ArchitectSkeletonGenerator.h"
#include "ArchitectTechStackSelector.h"
#include "SafetyAuditor.h"

#include <cctype>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

struct ScenarioTask {
    std::string moduleName;
    std::string functionName;
    std::string routeTo;   // deterministic | llm | human
    bool autoCompleted = false;
    bool reviewRequired = false;
    std::string contextBundle;
};

struct GreenfieldScenarioResult {
    std::string prompt;
    StructuredRequirements requirements;
    ModuleGraph graph;
    TechStackDecision stack;
    SkeletonProjectSpec skeleton;
    ScaffoldPlan scaffold;
    std::vector<ScenarioTask> tasks;
    std::vector<SafetyFinding> securityFindings;
    bool scaffoldApplied = false;
    std::vector<std::string> createdPaths;
    std::vector<std::string> notes;
};

class GreenfieldScenarioRunner {
public:
    static GreenfieldScenarioResult run(const std::string& workspaceRoot,
                                        const std::string& projectName = "url_shortener") {
        GreenfieldScenarioResult out;
        out.prompt = "Build a URL shortener with user accounts";

        out.requirements = ArchitectProblemParser::parse(
            "Build a URL shortener with user accounts, auth, REST API endpoints, CRUD links, "
            "PostgreSQL, and a TypeScript frontend.");
        out.graph = ArchitectModuleDecomposer::decompose(out.requirements, DecompositionStrategy::Layered);

        std::map<std::string, std::string> prefs{
            {"backend_language", "python"},
            {"database", "postgresql"},
            {"frontend_language", "typescript"}
        };
        out.stack = ArchitectTechStackSelector::select(out.requirements, out.graph, prefs);
        out.skeleton = ArchitectSkeletonGenerator::generate(out.requirements, out.graph, out.stack);
        out.scaffold = ArchitectScaffoldGenerator::buildPlan({
            workspaceRoot, projectName, out.skeleton, true
        });

        out.tasks = buildTasks(out.skeleton);
        runSecurityScan(out);
        materialize(workspaceRoot, out.scaffold, out);
        out.notes.push_back("Greenfield scenario executed end-to-end");
        return out;
    }

private:
    static std::vector<ScenarioTask> buildTasks(const SkeletonProjectSpec& skeleton) {
        std::vector<ScenarioTask> out;
        for (const auto& mod : skeleton.modules) {
            for (const auto& fn : mod.functions) {
                ScenarioTask t;
                t.moduleName = mod.moduleName;
                t.functionName = fn.name;
                routeTask(t);
                out.push_back(std::move(t));
            }
        }
        return out;
    }

    static void routeTask(ScenarioTask& t) {
        const std::string n = lower(t.functionName);
        const std::string m = lower(t.moduleName);

        if (m.find("auth") != std::string::npos || n.find("auth") != std::string::npos ||
            n.find("login") != std::string::npos || n.find("authorize") != std::string::npos) {
            t.routeTo = "human";
            t.reviewRequired = true;
            t.contextBundle = "Threat model + trust boundaries + auth policy";
            return;
        }

        if (isCrudLike(n) || m == "data" || m == "models") {
            t.routeTo = "deterministic";
            t.autoCompleted = true;
            t.contextBundle = "Schema + endpoint contract";
            return;
        }

        t.routeTo = "llm";
        t.contextBundle = "Module graph + function signature + related dependencies";
    }

    static bool isCrudLike(const std::string& n) {
        return contains(n, "create") || contains(n, "get") || contains(n, "list") ||
               contains(n, "update") || contains(n, "delete") || contains(n, "save") ||
               contains(n, "load") || contains(n, "validate");
    }

    static void runSecurityScan(GreenfieldScenarioResult& out) {
        std::string combined;
        for (const auto& f : out.scaffold.files) {
            if (f.relativePath.find("/src/") != std::string::npos ||
                f.relativePath.find("/internal/") != std::string::npos ||
                f.relativePath.find("/db/") != std::string::npos) {
                combined += "\n" + f.content;
            }
        }
        auto report = SafetyAuditor::audit(combined, "cpp", "generated_scenario");
        out.securityFindings = report.findings;
    }

    static void materialize(const std::string& workspaceRoot,
                            const ScaffoldPlan& plan,
                            GreenfieldScenarioResult& out) {
        std::string err;
        out.scaffoldApplied = ArchitectScaffoldGenerator::applyPlan(plan, &err);
        if (!out.scaffoldApplied) {
            out.notes.push_back("Scaffold apply failed: " + err);
            return;
        }

        for (const auto& f : plan.files) {
            const auto p = std::filesystem::path(workspaceRoot) / f.relativePath;
            if (std::filesystem::exists(p)) out.createdPaths.push_back(p.string());
        }
    }

    static std::string lower(std::string s) {
        for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static bool contains(const std::string& text, const std::string& needle) {
        return text.find(needle) != std::string::npos;
    }
};
