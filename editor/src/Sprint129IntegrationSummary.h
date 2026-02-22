#pragma once
// Step 1538: Sprint 129 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/DebugPolicyConstraint.h"
#include "debug/DebugActionBudget.h"
#include "debug/DebugRecoveryAdvisor.h"
#include "debug/DebugStopReasonClassifier.h"
#include "debug/DebugGuidancePacket.h"

struct Sprint129IntegrationResult {
    bool constraintsReady = false;
    bool budgetReady = false;
    bool recoveryReady = false;
    bool stopReasonReady = false;
    bool packetReady = false;
    bool constraintsToolReady = false;
    bool budgetToolReady = false;
    bool recoveryToolReady = false;
    bool stopReasonToolReady = false;
    bool success = false;
    int stepStart = 1529;
    int stepEnd = 1538;
    std::vector<std::string> filesAdded;
};

class Sprint129IntegrationSummary {
public:
    static Sprint129IntegrationResult run() {
        Sprint129IntegrationResult out;
        out.filesAdded = {
            "debug/DebugPolicyConstraint.h",
            "debug/DebugActionBudget.h",
            "debug/DebugRecoveryAdvisor.h",
            "debug/DebugStopReasonClassifier.h",
            "debug/DebugGuidancePacket.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint129IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto c = DebugPolicyConstraintModel::forMode("safe");
        out.constraintsReady = c.maxFilesTouched > 0;

        auto b = DebugActionBudgetModel::estimate("tiny", 2, 2);
        out.budgetReady = b.maxIterations > 0;

        auto r = DebugRecoveryAdvisor::advise("budget_exhausted");
        out.recoveryReady = !r.actions.empty();

        auto s = DebugStopReasonClassifier::classify(true, false, false, false);
        out.stopReasonReady = s.code == "budget_exhausted";

        out.packetReady = DebugGuidancePacketModel::toJson(DebugGuidancePacketModel::build(c, b, r, s)).contains("constraints");

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_get_debug_constraints") out.constraintsToolReady = true;
            if (n == "whetstone_estimate_debug_budget") out.budgetToolReady = true;
            if (n == "whetstone_get_recovery_advice") out.recoveryToolReady = true;
            if (n == "whetstone_classify_debug_stop_reason") out.stopReasonToolReady = true;
        }

        out.success = out.constraintsReady && out.budgetReady && out.recoveryReady && out.stopReasonReady && out.packetReady &&
                      out.constraintsToolReady && out.budgetToolReady && out.recoveryToolReady && out.stopReasonToolReady &&
                      out.stepStart == 1529 && out.stepEnd == 1538;
        return out;
    }
};
