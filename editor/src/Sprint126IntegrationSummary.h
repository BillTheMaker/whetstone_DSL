#pragma once
// Step 1508: Sprint 126 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/CampaignCheckpoint.h"
#include "debug/CampaignCheckpointStore.h"
#include "debug/CampaignQueuePolicy.h"
#include "debug/CampaignResumePlanner.h"
#include "debug/CampaignExportBundle.h"

struct Sprint126IntegrationResult {
    bool checkpointReady = false;
    bool storeReady = false;
    bool queueReady = false;
    bool resumeReady = false;
    bool exportReady = false;
    bool saveToolReady = false;
    bool listToolReady = false;
    bool resumeToolReady = false;
    bool exportToolReady = false;
    bool success = false;
    int stepStart = 1499;
    int stepEnd = 1508;
    std::vector<std::string> filesAdded;
};

class Sprint126IntegrationSummary {
public:
    static Sprint126IntegrationResult run() {
        Sprint126IntegrationResult out;
        out.filesAdded = {
            "debug/CampaignCheckpoint.h",
            "debug/CampaignCheckpointStore.h",
            "debug/CampaignQueuePolicy.h",
            "debug/CampaignResumePlanner.h",
            "debug/CampaignExportBundle.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint126IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        CampaignProgress p;
        p.campaignId = "c";
        p.totalTargets = 2;
        p.completedTargets = 1;
        auto cp = CampaignCheckpointModel::make("c", 1, p, "n");
        out.checkpointReady = cp.campaignId == "c" && cp.sequence == 1;

        const std::string path = "/tmp/whetstone_s126_checkpoint.json";
        out.storeReady = CampaignCheckpointStore::save(path, cp);

        auto ordered = CampaignQueuePolicy::order({{"a", 1, 10}, {"b", 2, 5}});
        out.queueReady = !ordered.empty() && ordered[0].campaignId == "b";

        auto plan = CampaignResumePlanner::plan("c", {cp});
        out.resumeReady = plan.canResume && plan.resumedFromSequence == 1;

        auto bundle = CampaignExportBundleModel::build(DebugCampaignSpecModel::make("n", {"t"}, "tiny", 3, true), p, {cp}, {{"k", 1}});
        out.exportReady = CampaignExportBundleModel::toJson(bundle).contains("campaign");

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_save_campaign_checkpoint") out.saveToolReady = true;
            if (n == "whetstone_list_campaign_checkpoints") out.listToolReady = true;
            if (n == "whetstone_resume_debug_campaign") out.resumeToolReady = true;
            if (n == "whetstone_export_debug_campaign_bundle") out.exportToolReady = true;
        }

        out.success = out.checkpointReady && out.storeReady && out.queueReady && out.resumeReady && out.exportReady &&
                      out.saveToolReady && out.listToolReady && out.resumeToolReady && out.exportToolReady &&
                      out.stepStart == 1499 && out.stepEnd == 1508;
        return out;
    }
};
