#pragma once

#include "../AgentChatPanelModel.h"
#include "../EditorState.h"
#include "../EditorUtils.h"

static void renderAgentChatPanel(EditorState& state) {
    if (!state.ui.showAgentChatPanel) return;
    if (!ImGui::Begin("Agent Chat", &state.ui.showAgentChatPanel)) {
        ImGui::End();
        return;
    }

    AgentChatState& chat = state.agent.chat;
    ImGui::BeginChild("##AgentChatHistory", ImVec2(0, -120), true);
    for (const auto& msg : chat.messages) {
        ImGui::Text("[%s] %s", msg.timestamp.c_str(), AgentChatPanelModel::roleLabel(msg.role));
        ImGui::SameLine();
        ImGui::TextWrapped("%s", msg.content.c_str());
        ImGui::Separator();
    }
    if (chat.autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 24.0f) {
        ImGui::SetScrollHereY(1.0f);
    }
    chat.autoScroll = false;
    ImGui::EndChild();

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    InputTextMultilineStr("##AgentChatInput", &chat.draftInput, ImVec2(-90, 80), flags);
    ImGui::SameLine();
    if (ImGui::Button("Send", ImVec2(80, 80))) {
        AgentChatPanelModel::sendUserMessage(&chat, "now");
    }

    ImGui::End();
}
