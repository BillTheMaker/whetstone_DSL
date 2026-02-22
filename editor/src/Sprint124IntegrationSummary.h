#pragma once
// Step 1488: Sprint 124 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/PatchExecutionRecord.h"
#include "debug/RollbackLedger.h"
#include "debug/BisectCandidateSelector.h"
#include "debug/PatchExecutionAuditBundle.h"

struct Sprint124IntegrationResult {
    bool recordReady = false;
    bool ledgerReady = false;
    bool bisectReady = false;
    bool validateToolReady = false;
    bool dryRunToolReady = false;
    bool applyToolReady = false;
    bool rollbackToolReady = false;
    bool bisectToolReady = false;
    bool success = false;
    int stepStart = 1479;
    int stepEnd = 1488;
    std::vector<std::string> filesAdded;
};

class Sprint124IntegrationSummary {
public:
    static Sprint124IntegrationResult run() {
        Sprint124IntegrationResult out;
        out.filesAdded = {
            "debug/PatchExecutionRecord.h",
            "debug/RollbackLedger.h",
            "debug/BisectCandidateSelector.h",
            "debug/PatchExecutionAuditBundle.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint124IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto rec = PatchExecutionRecordModel::make("p1", "validated", true, "ok");
        out.recordReady = !rec.executionId.empty();
        out.ledgerReady = RollbackLedger::append("/tmp/whetstone_s124_ledger.json", rec);
        auto ordered = BisectCandidateSelector::ordered({"p3", "p1", "p2"});
        out.bisectReady = !ordered.empty() && BisectCandidateSelector::midpoint(ordered).proposalId == "p2";

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_validate_patch_proposal") out.validateToolReady = true;
            if (n == "whetstone_dry_run_patch") out.dryRunToolReady = true;
            if (n == "whetstone_apply_patch_packet") out.applyToolReady = true;
            if (n == "whetstone_rollback_last_patch") out.rollbackToolReady = true;
            if (n == "whetstone_run_bisect_debug") out.bisectToolReady = true;
        }

        out.success = out.recordReady && out.ledgerReady && out.bisectReady && out.validateToolReady &&
                      out.dryRunToolReady && out.applyToolReady && out.rollbackToolReady && out.bisectToolReady &&
                      out.stepStart == 1479 && out.stepEnd == 1488;
        return out;
    }
};
