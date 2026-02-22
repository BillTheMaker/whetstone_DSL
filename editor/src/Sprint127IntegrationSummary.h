#pragma once
// Step 1518: Sprint 127 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/DebugHintTemplate.h"
#include "debug/FailureTriageScore.h"
#include "debug/MinimalReproReducer.h"
#include "debug/PatchRiskLabeler.h"
#include "debug/DebugAssistPacket.h"

struct Sprint127IntegrationResult {
    bool hintsReady = false;
    bool triageReady = false;
    bool reproReady = false;
    bool riskReady = false;
    bool packetReady = false;
    bool hintsToolReady = false;
    bool triageToolReady = false;
    bool reproToolReady = false;
    bool riskToolReady = false;
    bool success = false;
    int stepStart = 1509;
    int stepEnd = 1518;
    std::vector<std::string> filesAdded;
};

class Sprint127IntegrationSummary {
public:
    static Sprint127IntegrationResult run() {
        Sprint127IntegrationResult out;
        out.filesAdded = {
            "debug/DebugHintTemplate.h",
            "debug/FailureTriageScore.h",
            "debug/MinimalReproReducer.h",
            "debug/PatchRiskLabeler.h",
            "debug/DebugAssistPacket.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint127IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto hints = DebugHintTemplate::build("compile", {"src/main.cpp"});
        out.hintsReady = !hints.hints.empty();

        auto triage = FailureTriageScorer::compute("compile", 7, 8, 6);
        out.triageReady = triage.score > 0;

        auto repro = MinimalReproReducer::reduce("ctest --output-on-failure --verbose", {"--verbose"});
        out.reproReady = repro.removedFlags == 1;

        auto risk = PatchRiskLabeler::label(4, 40, false, false);
        out.riskReady = risk.level == "medium" || risk.level == "high";

        out.packetReady = DebugAssistPacketModel::toJson(DebugAssistPacketModel::build(hints, triage, repro, risk)).contains("triage");

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_generate_debug_hints") out.hintsToolReady = true;
            if (n == "whetstone_score_failure_triage") out.triageToolReady = true;
            if (n == "whetstone_reduce_repro_command") out.reproToolReady = true;
            if (n == "whetstone_label_patch_risk") out.riskToolReady = true;
        }

        out.success = out.hintsReady && out.triageReady && out.reproReady && out.riskReady && out.packetReady &&
                      out.hintsToolReady && out.triageToolReady && out.reproToolReady && out.riskToolReady &&
                      out.stepStart == 1509 && out.stepEnd == 1518;
        return out;
    }
};
