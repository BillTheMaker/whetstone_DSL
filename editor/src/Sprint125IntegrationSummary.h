#pragma once
// Step 1498: Sprint 125 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/DebugCampaignSpec.h"
#include "debug/CampaignTargetPrioritizer.h"
#include "debug/SafetyEnvelopePolicy.h"
#include "debug/CampaignProgressTracker.h"
#include "debug/CampaignReportBundle.h"

struct Sprint125IntegrationResult {
    bool specReady = false;
    bool prioritizerReady = false;
    bool safetyReady = false;
    bool progressReady = false;
    bool startToolReady = false;
    bool stepToolReady = false;
    bool statusToolReady = false;
    bool stopToolReady = false;
    bool success = false;
    int stepStart = 1489;
    int stepEnd = 1498;
    std::vector<std::string> filesAdded;
};

class Sprint125IntegrationSummary {
public:
    static Sprint125IntegrationResult run() {
        Sprint125IntegrationResult out;
        out.filesAdded = {
            "debug/DebugCampaignSpec.h",
            "debug/CampaignTargetPrioritizer.h",
            "debug/SafetyEnvelopePolicy.h",
            "debug/CampaignProgressTracker.h",
            "debug/CampaignReportBundle.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint125IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto spec = DebugCampaignSpecModel::make("c", {"b", "a"}, "tiny", 3, true);
        out.specReady = spec.targets[0] == "a";

        auto pri = CampaignTargetPrioritizer::prioritize({{"t1", 1, 1}, {"t2", 2, 1}});
        out.prioritizerReady = !pri.empty() && pri[0].target == "t2";

        auto safe = SafetyEnvelopePolicy::evaluate(SafetyEnvelope{}, 1, 0, 1000);
        out.safetyReady = safe.allowed;

        CampaignProgress p;
        p.campaignId = spec.campaignId;
        p.totalTargets = 2;
        p.completedTargets = 1;
        out.progressReady = CampaignProgressTracker::percent(p) == 50.0;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_start_debug_campaign") out.startToolReady = true;
            if (n == "whetstone_step_debug_campaign") out.stepToolReady = true;
            if (n == "whetstone_get_debug_campaign_status") out.statusToolReady = true;
            if (n == "whetstone_stop_debug_campaign") out.stopToolReady = true;
        }

        out.success = out.specReady && out.prioritizerReady && out.safetyReady && out.progressReady &&
                      out.startToolReady && out.stepToolReady && out.statusToolReady && out.stopToolReady &&
                      out.stepStart == 1489 && out.stepEnd == 1498;
        return out;
    }
};
