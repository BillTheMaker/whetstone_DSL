#pragma once
// Step 1468: Sprint 122 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/FailureFixtureCatalog.h"
#include "debug/PatchDryRunApplier.h"
#include "debug/PatchInvariantChecker.h"
#include "debug/DebugMetricsPacket.h"

struct Sprint122IntegrationResult {
    bool fixturesReady = false;
    bool dryRunReady = false;
    bool invariantsReady = false;
    bool clusterToolReady = false;
    bool contextToolReady = false;
    bool proposeToolReady = false;
    bool reproToolsReady = false;
    bool metricsReady = false;
    bool success = false;
    int stepStart = 1459;
    int stepEnd = 1468;
    std::vector<std::string> filesAdded;
};

class Sprint122IntegrationSummary {
public:
    static Sprint122IntegrationResult run() {
        Sprint122IntegrationResult out;
        out.filesAdded = {
            "debug/FailureFixtureCatalog.h",
            "debug/PatchDryRunApplier.h",
            "debug/PatchInvariantChecker.h",
            "debug/DebugMetricsPacket.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint122IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto fixtures = FailureFixtureCatalog::defaults();
        out.fixturesReady = !fixtures.empty() && FailureFixtureCatalog::find(fixtures, "compile_missing_include") != nullptr;

        auto dry = PatchDryRunApplier::run("diff --git a/editor/src/a.h b/editor/src/a.h\n--- a/editor/src/a.h\n+++ b/editor/src/a.h\n@@\n+// x\n");
        out.dryRunReady = dry.success;

        PatchProposal pp;
        pp.failureClusterId = "fc";
        pp.patchPlan.push_back({"intent", "editor/src/a.h"});
        pp.candidateDiff = "diff --git a/editor/src/a.h b/editor/src/a.h\n--- a/editor/src/a.h\n+++ b/editor/src/a.h\n@@\n+// x\n";
        pp.invariants = {"do_not_edit_unrelated_files"};
        pp.expectedTestsToRun = {"step1461_test"};
        auto inv = PatchInvariantChecker::check(pp, {"editor/src/a.h"});
        out.invariantsReady = inv.success;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        bool cluster = false, context = false, propose = false, save = false, replay = false;
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_cluster_failures") cluster = true;
            if (n == "whetstone_assemble_fix_context") context = true;
            if (n == "whetstone_propose_patch_for_failure") propose = true;
            if (n == "whetstone_save_repro_packet") save = true;
            if (n == "whetstone_replay_repro_packet") replay = true;
        }
        out.clusterToolReady = cluster;
        out.contextToolReady = context;
        out.proposeToolReady = propose;
        out.reproToolsReady = save && replay;

        auto metrics = DebugMetricsModel::compute("S122", 3, 1200, 800, 20, 5, 3, 4, 0, 4);
        out.metricsReady = metrics.patchAcceptanceRatePct == 75;

        out.success = out.fixturesReady && out.dryRunReady && out.invariantsReady && out.clusterToolReady &&
                      out.contextToolReady && out.proposeToolReady && out.reproToolsReady && out.metricsReady &&
                      out.stepStart == 1459 && out.stepEnd == 1468;
        return out;
    }
};
