#pragma once

#include "../EditorState.h"

static void renderHelpPanelWindow(EditorState& state) {
    if (!state.ui.showHelpPanel) return;
    ImGui::Begin("Documentation", &state.ui.showHelpPanel);
    ImGui::PushFont(state.uiFont);
    renderHelpPanel(state.helpPanel, state.workspaceRoot, state.monoFont);
    ImGui::PopFont();
    ImGui::End();
}
