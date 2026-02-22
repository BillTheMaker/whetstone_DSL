#pragma once
// Step 1478: Sprint 123 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/FailureTrendTracker.h"
#include "debug/PatchCandidateRanker.h"
#include "debug/ReproCommandRunner.h"
#include "debug/DebugBenchmarkHarness.h"
#include "debug/SLMDebugReadiness.h"

struct Sprint123IntegrationResult {
    bool trendReady = false;
    bool rankerReady = false;
    bool runnerReady = false;
    bool guardToolReady = false;
    bool exportToolReady = false;
    bool metricsToolReady = false;
    bool readinessToolReady = false;
    bool benchmarkReady = false;
    bool success = false;
    int stepStart = 1469;
    int stepEnd = 1478;
    std::vector<std::string> filesAdded;
};

class Sprint123IntegrationSummary {
public:
    static Sprint123IntegrationResult run() {
        Sprint123IntegrationResult out;
        out.filesAdded = {
            "debug/FailureTrendTracker.h",
            "debug/PatchCandidateRanker.h",
            "debug/ReproCommandRunner.h",
            "debug/DebugBenchmarkHarness.h",
            "debug/SLMDebugReadiness.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint123IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        out.trendReady = FailureTrendTracker::analyze({{1, 10}, {2, 8}}).improving;
        out.rankerReady = !PatchCandidateRanker::rank({{"p1", 0.9, 0, 0.0}}).empty();
        out.runnerReady = ReproCommandRunner::run("mock:ok").exitCode == 0;

        auto bench = DebugBenchmarkHarness::run(FailureFixtureCatalog::defaults());
        out.benchmarkReady = bench.success;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_regression_guard") out.guardToolReady = true;
            if (n == "whetstone_export_repro_jsonl") out.exportToolReady = true;
            if (n == "whetstone_get_debug_metrics") out.metricsToolReady = true;
            if (n == "whetstone_get_slm_debug_readiness") out.readinessToolReady = true;
        }

        out.success = out.trendReady && out.rankerReady && out.runnerReady && out.benchmarkReady &&
                      out.guardToolReady && out.exportToolReady && out.metricsToolReady && out.readinessToolReady &&
                      out.stepStart == 1469 && out.stepEnd == 1478;
        return out;
    }
};
