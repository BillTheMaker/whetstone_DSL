#pragma once
#include <memory>
#include <vector>
#include <map>
#include "WebSocketServer.h"
#include "WorkflowRecorder.h"
#include "AgentRegistry.h"
#include "AgentMarketplace.h"
#include "AgentPermissionPolicy.h"
#include "../AgentChatPanelModel.h"
#include "../AgentTaskSlots.h"

struct AgentState {
    std::unique_ptr<WebSocketAgentServer> server;
    MockWebSocketTransport* transport = nullptr;
    int port = 8765;
    AgentRole defaultRole = AgentRole::Linter;
    std::vector<std::string> log;
    std::map<std::string, AgentRole> roles;
    WorkflowRecorder workflowRecorder;
    AgentRegistry registry;
    AgentMarketplaceState marketplace;
    AgentChatState chat;
    AgentTaskSlotsState taskSlots;
};
