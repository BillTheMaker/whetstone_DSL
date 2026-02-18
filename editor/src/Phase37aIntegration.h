#pragma once
// Step 628: Phase 37a integration

#include "AgentChatPanelModel.h"
#include "AgentMutationApproval.h"

#include <string>

struct Phase37aIntegrationResult {
    bool sentUserMessage = false;
    bool toolCallShown = false;
    bool previewShown = false;
    bool accepted = false;
    bool astUpdated = false;
    std::string activeCode;
};

class Phase37aIntegration {
public:
    static Phase37aIntegrationResult run(const std::string& spec,
                                         const std::string& beforeCode,
                                         const std::string& generatedCode) {
        Phase37aIntegrationResult out;
        AgentChatState chat;
        chat.draftInput = spec;
        out.sentUserMessage = AgentChatPanelModel::sendUserMessage(&chat, "12:00");

        AgentChatPanelModel::addToolCall(&chat,
                                         "whetstone_generate_code",
                                         {{"spec", spec}},
                                         {{"generatedCode", generatedCode}},
                                         "12:01");
        out.toolCallShown = !chat.toolCalls.empty();

        AgentChatPanelModel::addMutationPreview(&chat,
                                                "preview-1",
                                                "{\"type\":\"replaceModule\"}",
                                                beforeCode,
                                                generatedCode,
                                                "12:02");
        out.previewShown = !chat.mutationPreviews.empty();

        out.accepted = AgentMutationApproval::accept(&chat.mutationApprovals, "preview-1");
        out.activeCode = beforeCode;
        if (out.accepted) {
            out.activeCode = generatedCode;
            out.astUpdated = true;
        }
        return out;
    }
};
