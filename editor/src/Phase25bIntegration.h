#pragma once

// Step 503: Phase 25b Integration
// Integrates the four end-to-end scenario runners from steps 499-502.

#include "CrossLanguagePortScenarioRunner.h"
#include "GreenfieldScenarioRunner.h"
#include "LegacyModernizationScenarioRunner.h"
#include "MultiModelOrchestrationScenarioRunner.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

struct Phase25bEvent {
    std::string scenario;
    std::string stage;
    std::string detail;
};

struct ScenarioCostSnapshot {
    std::string scenario;
    int estimatedTokens = 0;
    int actualTokens = 0;

    int delta() const { return std::abs(estimatedTokens - actualTokens); }
};

struct Phase25bIntegrationResult {
    GreenfieldScenarioResult greenfield;
    LegacyModernizationScenarioResult legacy;
    CrossLanguagePortScenarioResult crossLanguage;
    MultiModelOrchestrationScenarioResult multiModel;

    std::vector<ScenarioCostSnapshot> costs;
    std::vector<Phase25bEvent> events;
    std::map<std::string, bool> scenarioPass;
    int totalSecurityFindings = 0;
    bool allScenariosPassed = false;
    bool uniqueWorkflowPathsObserved = false;
    bool costTrackingAccurate = false;
    bool eventStreamComplete = false;
    std::vector<std::string> notes;
};

class Phase25bIntegration {
public:
    static Phase25bIntegrationResult run() {
        Phase25bIntegrationResult out;

        emit(out, "greenfield", "start", "execute scenario");
        out.greenfield = GreenfieldScenarioRunner::run(tempRoot(), "phase25b_greenfield");
        emit(out, "greenfield", "complete", "tasks=" + std::to_string(out.greenfield.tasks.size()));

        emit(out, "legacy", "start", "execute scenario");
        out.legacy = LegacyModernizationScenarioRunner::run();
        emit(out, "legacy", "complete",
             "suggestions=" + std::to_string(out.legacy.modernizationReport.suggestions.size()));

        emit(out, "cross-language", "start", "execute scenario");
        out.crossLanguage = CrossLanguagePortScenarioRunner::run();
        emit(out, "cross-language", "complete",
             "functions=" + std::to_string(out.crossLanguage.functions.size()));

        emit(out, "multi-model", "start", "execute scenario");
        out.multiModel = MultiModelOrchestrationScenarioRunner::run();
        emit(out, "multi-model", "complete",
             "tasks=" + std::to_string(out.multiModel.tasks.size()));

        evaluatePassFlags(out);
        buildCostSnapshots(out);
        evaluateIntegrationSignals(out);

        out.notes.push_back("Phase 25b integration pipeline executed");
        out.notes.push_back(out.allScenariosPassed ? "all scenarios passed" : "one or more scenarios failed");
        return out;
    }

private:
    static void emit(Phase25bIntegrationResult& out,
                     const std::string& scenario,
                     const std::string& stage,
                     const std::string& detail) {
        out.events.push_back({scenario, stage, detail});
    }

    static std::string tempRoot() {
        return "/tmp";
    }

    static void evaluatePassFlags(Phase25bIntegrationResult& out) {
        out.scenarioPass["greenfield"] =
            out.greenfield.scaffoldApplied && !out.greenfield.createdPaths.empty();
        out.scenarioPass["legacy"] =
            !out.legacy.modernizationReport.suggestions.empty() &&
            out.legacy.migrationPlan.targetLanguage == "rust";
        out.scenarioPass["cross-language"] =
            out.crossLanguage.sourceLanguage == "python" &&
            out.crossLanguage.targetLanguage == "rust" &&
            !out.crossLanguage.functions.empty();
        out.scenarioPass["multi-model"] =
            out.multiModel.tasks.size() == 20 && out.multiModel.reviewGatesExercised;

        out.allScenariosPassed = true;
        for (const auto& kv : out.scenarioPass) {
            if (!kv.second) out.allScenariosPassed = false;
        }
    }

    static void buildCostSnapshots(Phase25bIntegrationResult& out) {
        ScenarioCostSnapshot g;
        g.scenario = "greenfield";
        g.estimatedTokens = static_cast<int>(out.greenfield.tasks.size()) * 500;
        g.actualTokens = estimateGreenfieldActual(out.greenfield);
        out.costs.push_back(g);

        ScenarioCostSnapshot l;
        l.scenario = "legacy";
        l.estimatedTokens = static_cast<int>(out.legacy.modernizationWorkflow.items.size()) * 550;
        l.actualTokens = estimateLegacyActual(out.legacy);
        out.costs.push_back(l);

        ScenarioCostSnapshot c;
        c.scenario = "cross-language";
        c.estimatedTokens = static_cast<int>(out.crossLanguage.functions.size()) * 700;
        c.actualTokens = estimateCrossLanguageActual(out.crossLanguage);
        out.costs.push_back(c);

        ScenarioCostSnapshot m;
        m.scenario = "multi-model";
        m.estimatedTokens = out.multiModel.estimatedTotalTokens;
        m.actualTokens = out.multiModel.actualTotalTokens;
        out.costs.push_back(m);
    }

    static void evaluateIntegrationSignals(Phase25bIntegrationResult& out) {
        out.totalSecurityFindings =
            static_cast<int>(out.greenfield.securityFindings.size()) +
            static_cast<int>(out.legacy.safetyReport.findings.size());

        out.uniqueWorkflowPathsObserved =
            hasUniquePathGreenfield(out.greenfield) &&
            hasUniquePathLegacy(out.legacy) &&
            hasUniquePathCrossLanguage(out.crossLanguage) &&
            hasUniquePathMultiModel(out.multiModel);

        out.costTrackingAccurate = true;
        for (const auto& c : out.costs) {
            if (c.estimatedTokens <= 0 || c.actualTokens < 0) {
                out.costTrackingAccurate = false;
                break;
            }
            if (c.delta() > c.estimatedTokens) {
                out.costTrackingAccurate = false;
                break;
            }
        }

        out.eventStreamComplete = true;
        for (const auto& name : {"greenfield", "legacy", "cross-language", "multi-model"}) {
            bool hasStart = false;
            bool hasComplete = false;
            for (const auto& e : out.events) {
                if (e.scenario != name) continue;
                if (e.stage == "start") hasStart = true;
                if (e.stage == "complete") hasComplete = true;
            }
            if (!hasStart || !hasComplete) out.eventStreamComplete = false;
        }
    }

    static int estimateGreenfieldActual(const GreenfieldScenarioResult& r) {
        int total = 0;
        for (const auto& t : r.tasks) {
            if (t.routeTo == "deterministic") total += 0;
            else if (t.routeTo == "llm") total += 700;
            else total += 0;
        }
        return total;
    }

    static int estimateLegacyActual(const LegacyModernizationScenarioResult& r) {
        int total = 0;
        for (const auto& i : r.modernizationWorkflow.items) {
            if (i.routing == WorkItemRouting::Deterministic) total += 80;
            else if (i.routing == WorkItemRouting::LLM) total += 500;
            else total += 0;
        }
        return total;
    }

    static int estimateCrossLanguageActual(const CrossLanguagePortScenarioResult& r) {
        int total = 0;
        for (const auto& f : r.functions) {
            if (f.confidence.reviewRequired) total += 800;
            else total += 450;
        }
        return total;
    }

    static bool hasUniquePathGreenfield(const GreenfieldScenarioResult& r) {
        bool hasDet = false, hasLlm = false, hasHuman = false;
        for (const auto& t : r.tasks) {
            if (t.routeTo == "deterministic") hasDet = true;
            if (t.routeTo == "llm") hasLlm = true;
            if (t.routeTo == "human") hasHuman = true;
        }
        return hasDet && hasLlm && hasHuman;
    }

    static bool hasUniquePathLegacy(const LegacyModernizationScenarioResult& r) {
        return r.modernizationWorkflow.countByRouting(WorkItemRouting::Deterministic) > 0 &&
               r.modernizationWorkflow.countByRouting(WorkItemRouting::LLM) > 0;
    }

    static bool hasUniquePathCrossLanguage(const CrossLanguagePortScenarioResult& r) {
        return !r.reviewRequiredFunctions.empty() &&
               r.concurrencyAnalysis.hasPattern("async/await");
    }

    static bool hasUniquePathMultiModel(const MultiModelOrchestrationScenarioResult& r) {
        return r.routedCounts.count("deterministic") &&
               r.routedCounts.count("template") &&
               r.routedCounts.count("slm") &&
               r.routedCounts.count("llm") &&
               r.routedCounts.count("human");
    }
};
