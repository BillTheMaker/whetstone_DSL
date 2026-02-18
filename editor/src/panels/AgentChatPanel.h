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

    if (!chat.mutationPreviews.empty()) {
        ImGui::Separator();
        ImGui::TextUnformatted("Mutation Previews");
        for (std::size_t i = 0; i < chat.mutationPreviews.size(); ++i) {
            const auto& preview = chat.mutationPreviews[i];
            AgentMutationApproval::ensureRecord(&chat.mutationApprovals, preview.previewId);
            const MutationApprovalRecord* approval =
                AgentMutationApproval::find(chat.mutationApprovals, preview.previewId);
            std::string header = preview.previewId + (preview.hasChanges ? " (changes)" : " (no changes)");
            if (ImGui::CollapsingHeader((header + "##preview_" + std::to_string(i)).c_str())) {
                ImGui::TextUnformatted("Mutation JSON");
                ImGui::BeginChild(("##preview_json_" + std::to_string(i)).c_str(), ImVec2(-1, 60), true);
                ImGui::TextUnformatted(preview.mutationJson.c_str());
                ImGui::EndChild();

                ImGui::Columns(2, ("##preview_cols_" + std::to_string(i)).c_str(), true);
                ImGui::TextUnformatted("Before");
                ImGui::BeginChild(("##preview_before_" + std::to_string(i)).c_str(), ImVec2(0, 100), true);
                ImGui::TextUnformatted(preview.beforeCode.c_str());
                ImGui::EndChild();
                ImGui::NextColumn();
                ImGui::TextUnformatted("After");
                ImGui::BeginChild(("##preview_after_" + std::to_string(i)).c_str(), ImVec2(0, 100), true);
                ImGui::TextUnformatted(preview.afterCode.c_str());
                ImGui::EndChild();
                ImGui::Columns(1);

                ImGui::Separator();
                if (approval) {
                    ImGui::Text("Decision: %s", AgentMutationApproval::decisionText(approval->decision).c_str());
                }
                if (ImGui::Button(("Accept##" + preview.previewId).c_str())) {
                    AgentMutationApproval::accept(&chat.mutationApprovals, preview.previewId);
                }
                ImGui::SameLine();
                if (ImGui::Button(("Reject##" + preview.previewId).c_str())) {
                    AgentMutationApproval::reject(&chat.mutationApprovals, preview.previewId, "user_rejected");
                }
                ImGui::SameLine();
                if (ImGui::Button(("Modify##" + preview.previewId).c_str())) {
                    AgentMutationApproval::modify(&chat.mutationApprovals,
                                                  preview.previewId,
                                                  preview.mutationJson);
                }
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
