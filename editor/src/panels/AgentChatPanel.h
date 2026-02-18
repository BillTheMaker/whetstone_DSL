#pragma once

#include "../AgentChatPanelModel.h"
#include "../AgentChatContextInjector.h"
#include "../AgentChatSessionPersistence.h"
#include "../AgentTaskStatusOverlay.h"
#include "../EditorState.h"
#include "../EditorUtils.h"
#include <ctime>

static int countChatContextNodes(const ASTNode* node) {
    if (!node) return 0;
    int total = 1;
    for (const auto* child : node->allChildren()) total += countChatContextNodes(child);
    return total;
}

static void renderAgentChatPanel(EditorState& state) {
    if (!state.ui.showAgentChatPanel) return;
    if (!ImGui::Begin("Agent Chat", &state.ui.showAgentChatPanel)) {
        ImGui::End();
        return;
    }

    AgentChatState& chat = state.agent.chat;
    AgentTaskSlotsState& taskSlots = state.agent.taskSlots;
    const std::string projectFile = state.activeBuffer ? state.activeBuffer->path : "";
    if (!projectFile.empty() &&
        (!chat.sessionLoaded || chat.loadedProjectFile != projectFile)) {
        AgentChatSavedSession loaded;
        std::string error;
        if (AgentChatSessionPersistence::loadLatestSession(state.workspaceRoot,
                                                           projectFile,
                                                           &loaded,
                                                           &error)) {
            chat = loaded.chat;
            chat.loadedProjectFile = projectFile;
            chat.sessionLoaded = true;
            chat.persistedMessageCount = chat.messages.size();
        } else {
            chat.loadedProjectFile = projectFile;
            chat.sessionLoaded = true;
        }
    }

    if (chat.systemContext.empty()) {
        AgentChatRuntimeSnapshot snapshot;
        if (state.activeBuffer) {
            snapshot.activeFile = state.activeBuffer->path;
            snapshot.language = state.activeBuffer->language;
        }
        snapshot.workspacePath = state.workspaceRoot.empty() ? "." : state.workspaceRoot;
        Module* ast = state.activeAST();
        snapshot.astNodeCount = countChatContextNodes(ast);
        snapshot.openAnnotationCount = countAnnotationNodes(ast);
        AgentChatContextInjector::inject(&chat, snapshot, "context");
    }
    (void)AgentTaskSlots::assignPendingToSlots(&taskSlots);
    const int nowSeconds = static_cast<int>(std::time(nullptr));
    const auto overlayRows = AgentTaskStatusOverlay::buildRows(taskSlots, nowSeconds);

    if (!overlayRows.empty() && ImGui::BeginTabBar("##AgentTaskSlotsTabs")) {
        for (const auto& row : overlayRows) {
            std::string tabName = (row.slotIndex >= 0)
                ? ("Slot " + std::to_string(row.slotIndex + 1) + ": " + row.taskId)
                : ("Pending: " + row.taskId);
            if (ImGui::BeginTabItem(tabName.c_str())) {
                ImGui::Text("Task: %s", row.description.c_str());
                ImGui::Text("Status: %s", row.status.c_str());
                ImGui::Text("Elapsed: %ds", row.elapsedSeconds);
                ImGui::Text("Tool Calls: %d", row.toolCallsMade);
                ImGui::Text("Current Step: %s", row.currentStep.c_str());
                if (row.canCancel &&
                    ImGui::Button(("Cancel##tab_" + row.taskId).c_str())) {
                    (void)AgentTaskStatusOverlay::cancelTask(&taskSlots, row.taskId);
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

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

    if (!overlayRows.empty()) {
        ImGui::Separator();
        ImGui::TextUnformatted("Agent Tasks");
        for (const auto& row : overlayRows) {
            ImGui::Text("[%s] %s", row.status.c_str(), row.description.c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("(elapsed: %ds, tools: %d, step: %s)",
                                row.elapsedSeconds,
                                row.toolCallsMade,
                                row.currentStep.c_str());
            if (row.canCancel &&
                ImGui::Button(("Cancel##row_" + row.taskId).c_str())) {
                (void)AgentTaskStatusOverlay::cancelTask(&taskSlots, row.taskId);
            }
        }
    }

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    InputTextMultilineStr("##AgentChatInput", &chat.draftInput, ImVec2(-90, 80), flags);
    ImGui::SameLine();
    if (ImGui::Button("Send", ImVec2(80, 80))) {
        AgentChatPanelModel::sendUserMessage(&chat, "now");
    }

    if (!projectFile.empty() &&
        chat.sessionLoaded &&
        chat.messages.size() != chat.persistedMessageCount) {
        std::string error;
        if (AgentChatSessionPersistence::saveSession(state.workspaceRoot,
                                                     projectFile,
                                                     "autosave",
                                                     chat,
                                                     &error)) {
            chat.persistedMessageCount = chat.messages.size();
        }
    }

    ImGui::End();
}
