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

    if (!chat.toolCalls.empty()) {
        ImGui::Separator();
        ImGui::TextUnformatted("Tool Calls");
        for (std::size_t i = 0; i < chat.toolCalls.size(); ++i) {
            auto& call = chat.toolCalls[i];
            std::string label = AgentToolCallVisualization::inlineLabel(call);
            if (ImGui::CollapsingHeader((label + "##tool_" + std::to_string(i)).c_str())) {
                call.expanded = true;
                ImGui::TextUnformatted("Input");
                ImGui::BeginChild(("##tool_in_" + std::to_string(i)).c_str(), ImVec2(-1, 70), true);
                ImGui::TextUnformatted(call.inputJson.c_str());
                ImGui::EndChild();
                ImGui::TextUnformatted("Output");
                ImGui::BeginChild(("##tool_out_" + std::to_string(i)).c_str(), ImVec2(-1, 70), true);
                ImGui::TextUnformatted(call.outputJson.c_str());
                ImGui::EndChild();
            } else {
                call.expanded = false;
            }
        }
    }

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    InputTextMultilineStr("##AgentChatInput", &chat.draftInput, ImVec2(-90, 80), flags);
    ImGui::SameLine();
    if (ImGui::Button("Send", ImVec2(80, 80))) {
        AgentChatPanelModel::sendUserMessage(&chat, "now");
    }

    ImGui::End();
}
