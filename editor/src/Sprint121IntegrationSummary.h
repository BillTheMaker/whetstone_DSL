#pragma once
// Step 1458: Sprint 121 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/FailurePacket.h"
#include "debug/FailureClusterer.h"
#include "debug/FixContextAssembler.h"
#include "debug/FailurePatchProposer.h"
#include "debug/ReproPacketStore.h"
#include "debug/RegressionGuardPlanner.h"
#include "debug/DebugLoopOrchestrator.h"

struct Sprint121IntegrationResult {
    bool packetReady = false;
    bool clusterReady = false;
    bool contextReady = false;
    bool proposerReady = false;
    bool reproReady = false;
    bool guardReady = false;
    bool loopReady = false;
    bool toolsReady = false;
    bool success = false;
    int stepStart = 1449;
    int stepEnd = 1458;
    std::vector<std::string> filesAdded;
};

class Sprint121IntegrationSummary {
public:
    static Sprint121IntegrationResult run() {
        Sprint121IntegrationResult out;
        out.filesAdded = {
            "debug/FailurePacket.h",
            "debug/FailureClusterer.h",
            "debug/FixContextAssembler.h",
            "debug/PatchProposal.h",
            "debug/FailurePatchProposer.h",
            "debug/ReproPacketStore.h",
            "debug/RegressionGuardPlanner.h",
            "debug/DebugLoopOrchestrator.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint121IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto p = FailurePacketModel::make("mock:error: src/a.cpp:10:1 missing", "error: src/a.cpp:10:1 missing", 1);
        out.packetReady = !p.packetId.empty();

        auto c = FailureClusterer::cluster({p});
        out.clusterReady = !c.empty() && c[0].fixFirst;

        auto ctx = FixContextAssembler::assemble("tiny", {{"src/a.cpp", 10, 15, "line"}});
        out.contextReady = ctx.totalChars > 0;

        auto s = FailurePatchProposer::propose(c[0], ctx, "step1453_test");
        out.proposerReady = !s.proposal.proposalId.empty();

        ReproPacket rp;
        rp.failure = p;
        rp.envHash = "env";
        rp.reproCommand = p.reproCommand;
        const std::string path = "/tmp/whetstone_s121_repro.json";
        out.reproReady = ReproPacketStore::save(path, rp);

        auto g = RegressionGuardPlanner::plan({"editor/src/MCPServer.h"}, 1458, "step1458_test");
        out.guardReady = !g.mustPass.empty();

        auto loop = DebugLoopOrchestrator::run("mock:fixable compile error", 2, "tiny", true);
        out.loopReady = (loop.status == "green");

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        bool capture = false, debug = false;
        for (const auto& t : list["result"]["tools"]) {
            const std::string n = t.value("name", "");
            if (n == "whetstone_capture_failure_packet") capture = true;
            if (n == "whetstone_debug_until_green") debug = true;
        }
        out.toolsReady = capture && debug;

        out.success = out.packetReady && out.clusterReady && out.contextReady &&
                      out.proposerReady && out.reproReady && out.guardReady &&
                      out.loopReady && out.toolsReady &&
                      out.stepStart == 1449 && out.stepEnd == 1458;
        return out;
    }
};
