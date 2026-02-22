#pragma once
// Step 1548: Sprint 130 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/DebugChecklistItem.h"
#include "debug/DebugChecklistTemplate.h"
#include "debug/DebugSessionHandoff.h"
#include "debug/DebugEvidenceIndex.h"
#include "debug/DebugHandoffPacket.h"

struct Sprint130IntegrationResult {
    bool itemReady = false;
    bool checklistReady = false;
    bool handoffReady = false;
    bool evidenceReady = false;
    bool packetReady = false;
    bool checklistToolReady = false;
    bool markToolReady = false;
    bool handoffToolReady = false;
    bool evidenceToolReady = false;
    bool success = false;
    int stepStart = 1539;
    int stepEnd = 1548;
    std::vector<std::string> filesAdded;
};

class Sprint130IntegrationSummary {
public:
    static Sprint130IntegrationResult run() {
        Sprint130IntegrationResult out;
        out.filesAdded = {
            "debug/DebugChecklistItem.h",
            "debug/DebugChecklistTemplate.h",
            "debug/DebugSessionHandoff.h",
            "debug/DebugEvidenceIndex.h",
            "debug/DebugHandoffPacket.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint130IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto item = DebugChecklistItemModel::make(1, "x", false);
        out.itemReady = item.index == 1;

        auto chk = DebugChecklistTemplateModel::build("compile");
        out.checklistReady = !chk.items.empty();

        auto handoff = DebugSessionHandoffModel::build("s", "sum", {"next"});
        out.handoffReady = !handoff.summary.empty();

        auto ev = DebugEvidenceIndexModel::sort({{"trace", "t1"}, {"checklist_item", "c1"}});
        out.evidenceReady = !ev.empty();

        out.packetReady = DebugHandoffPacketModel::toJson(DebugHandoffPacketModel::build(chk, handoff, ev)).contains("handoff");

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_get_debug_checklist") out.checklistToolReady = true;
            if (n == "whetstone_mark_debug_checklist_item") out.markToolReady = true;
            if (n == "whetstone_build_debug_handoff") out.handoffToolReady = true;
            if (n == "whetstone_get_debug_evidence_index") out.evidenceToolReady = true;
        }

        out.success = out.itemReady && out.checklistReady && out.handoffReady && out.evidenceReady && out.packetReady &&
                      out.checklistToolReady && out.markToolReady && out.handoffToolReady && out.evidenceToolReady &&
                      out.stepStart == 1539 && out.stepEnd == 1548;
        return out;
    }
};
