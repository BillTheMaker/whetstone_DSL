#pragma once
// Step 658: Phase 40a integration

#include "ApiaryBrowserPanelModel.h"
#include "EnergyContextStatusModel.h"
#include "HiveMindJobPublisher.h"
#include "SwarmStatusPanelModel.h"

struct Phase40aIntegrationResult {
    bool energyVisible = false;
    bool jobDispatched = false;
    bool swarmStatusVisible = false;
    bool apiaryVisible = false;
    bool integrationReady = false;
};

class Phase40aIntegration {
public:
    static Phase40aIntegrationResult run() {
        Phase40aIntegrationResult out;
        EnergyContextState energy{"HighSolar", 450, 85};
        out.energyVisible = EnergyContextStatusModel::statusLabel(energy) == "HighSolar 450W";

        auto dispatch = HiveMindJobPublisher::dispatch({{"id", "job-40a-1"}, {"type", "generate"}});
        out.jobDispatched = dispatch.queuedInNexus && dispatch.publishedToTopic;

        auto swarm = SwarmStatusPanelModel::summarize({{"job-40a-1", "nodeA", 4, SwarmJobState::Running}});
        out.swarmStatusVisible = swarm.running == 1;

        std::vector<ApiaryToolEntry> tools = {
            {"drone", "1.0", "rust", {"jobs"}, "tool", "/mnt/nas/swarm_tools/drone.rs"}
        };
        out.apiaryVisible = !ApiaryBrowserPanelModel::filterByCapability(tools, "jobs").empty();

        out.integrationReady = out.energyVisible && out.jobDispatched && out.swarmStatusVisible && out.apiaryVisible;
        return out;
    }
};
