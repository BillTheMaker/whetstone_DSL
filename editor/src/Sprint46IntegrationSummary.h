#pragma once
// Step 697: Sprint 46 integration summary

#include <algorithm>
#include <string>
#include <vector>

#include "LanguageCapabilityMatrix.h"
#include "LanguageSupportTier.h"
#include "MCPServer.h"
#include "MigrationAcceptanceContract.h"
#include "Sprint46DocsBaseline.h"

struct Sprint46IntegrationResult {
    bool semanticCoreReachable = false;
    bool supportTierReachable = false;
    bool capabilityMatrixReachable = false;
    bool migrationContractReachable = false;
    bool docsBaselineReachable = false;
    bool allModulesReachable = false;

    bool languageMatrixToolListed = false;
    bool portingContractToolListed = false;

    int stepStart = 689;
    int stepEnd = 697;

    std::vector<std::string> filesAdded;
    bool regressionMarkerEmitted = false;
    std::string regressionMarker;

    bool success = false;
};

class Sprint46IntegrationSummary {
public:
    static Sprint46IntegrationResult run() {
        Sprint46IntegrationResult out;
        out.filesAdded = {
            "SemanticCoreIR.h",
            "LanguageSupportTier.h",
            "LanguageCapabilityMatrix.h",
            "MigrationAcceptanceContract.h",
            "LanguageToIRAdapter.h",
            "IRToLanguageAdapter.h",
            "RegisterPortingFoundationTools.h",
            "Sprint46DocsBaseline.h",
            "SPRINT46_FOUNDATION.md",
            "sprint46_semantic_core_example.json",
            "Sprint46IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto ir = Sprint46DocsBaseline::exampleIr();
        std::string irErr;
        out.semanticCoreReachable = SemanticCoreIRModel::validate(ir, &irErr);

        auto stable = LanguageSupportTierModel::requiredGates(SupportTier::Stable);
        out.supportTierReachable = stable.parseAndProject &&
                                   stable.executableEquivalence &&
                                   stable.staticChecks &&
                                   stable.securityGates &&
                                   stable.performanceGates &&
                                   stable.migrationReportQuality;

        auto rows = LanguageCapabilityMatrix::defaultRows();
        const auto* rust = LanguageCapabilityMatrix::get(rows, "rust");
        out.capabilityMatrixReachable = !rows.empty() && rust != nullptr;

        MigrationEvidence ev;
        ev.buildSuccess = true;
        ev.testsProvided = true;
        ev.testPassRate = 1.0f;
        ev.securityReportProvided = true;
        ev.highSeverityFindings = 0;
        ev.benchmarkProvided = true;
        ev.perfRegressionPct = 2.0f;
        ev.checklistComplete = true;
        auto gate = MigrationAcceptanceContract::evaluate(ev, MigrationGateThresholds{});
        out.migrationContractReachable = gate.pass;

        out.docsBaselineReachable = !Sprint46DocsBaseline::docPaths().empty() &&
                                    Sprint46DocsBaseline::exampleContractPacket().contains("minimumTestPassRate");

        out.allModulesReachable = out.semanticCoreReachable &&
                                  out.supportTierReachable &&
                                  out.capabilityMatrixReachable &&
                                  out.migrationContractReachable &&
                                  out.docsBaselineReachable;

        MCPServer mcp;
        json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}};
        json resp = mcp.handleRequest(req);
        for (const auto& t : resp["result"]["tools"]) {
            const std::string name = t.value("name", "");
            if (name == "whetstone_get_language_matrix") out.languageMatrixToolListed = true;
            if (name == "whetstone_get_porting_contract") out.portingContractToolListed = true;
        }

        out.regressionMarker = out.allModulesReachable &&
                               out.languageMatrixToolListed &&
                               out.portingContractToolListed
                               ? "sprint46_foundation_ready"
                               : "sprint46_foundation_blocked";
        out.regressionMarkerEmitted = !out.regressionMarker.empty();

        out.success = out.allModulesReachable &&
                      out.languageMatrixToolListed &&
                      out.portingContractToolListed &&
                      out.stepStart == 689 &&
                      out.stepEnd == 697 &&
                      out.regressionMarker == "sprint46_foundation_ready";
        return out;
    }
};
