#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"

static void renderExplorerPanel(EditorState& state) {
    if (state.ui.focusTarget == FocusRegion::Explorer) {
        ImGui::SetNextWindowFocus();
        state.ui.focusTarget = FocusRegion::None;
    }
    ImGui::Begin("Explorer");
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        state.ui.focusedRegion = FocusRegion::Explorer;
    }
    ImGui::PushFont(state.uiFont);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "OPEN EDITORS");
    ImGui::Separator();
    for (const auto& path : state.buffers.getOpenBuffers()) {
        auto* buf = state.bufferStates[path].get();
        std::string label = path;
        if (buf && buf->modified) label += " *";
        bool selected = state.active() && state.active()->path == path;
        if (ImGui::Selectable(label.c_str(), selected)) {
            state.switchToBuffer(path);
        }
    }
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "FILES");
    ImGui::Separator();
    state.refreshFileTree();
    if (state.fileTreeRoot.path.empty()) {
        ImGui::TextDisabled("(no workspace)");
        ImGui::Spacing();
        ImGui::TextWrapped("Open a folder to browse project files.");
        if (ImGui::Button("Open Folder")) {
            auto path = FileDialog::openFolder({"Open Folder", state.workspaceRoot});
            if (!path.empty()) {
                std::string error;
                if (!state.setWorkspaceRoot(path, &error)) {
                    state.notify(NotificationLevel::Error,
                                 "Failed to open folder: " + error);
                } else {
                    state.lastDialogPath = path;
                }
            }
        }
    } else {
        RenderFileTree(state.fileTreeRoot, state);
    }
    ImGui::PopFont();
    ImGui::End();
}
