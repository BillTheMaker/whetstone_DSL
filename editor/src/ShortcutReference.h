#pragma once

#include "imgui.h"
#include "EditorState.h"

#include <algorithm>
#include <string>

struct ShortcutReferenceState {
    char filter[128] = {};
};

static void renderShortcutReference(EditorState& state, ShortcutReferenceState& panel) {
    if (!state.ui.showShortcutReference) return;
    ImGui::Begin("Keyboard Shortcuts", &state.ui.showShortcutReference);
    ImGui::PushFont(state.uiFont);
    ImGui::InputText("Filter", panel.filter, sizeof(panel.filter));
    std::string filterText = panel.filter;
    std::transform(filterText.begin(), filterText.end(), filterText.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    ImGui::Separator();

    ImGui::BeginChild("##shortcuts", ImVec2(0, 0), false);
    auto actions = state.keys.listActions();
    std::sort(actions.begin(), actions.end());
    for (const auto& action : actions) {
        std::string lower = action;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        if (!filterText.empty() && lower.find(filterText) == std::string::npos) continue;
        LegacyKeyCombo combo = state.keys.getBinding(action);
        ImGui::Text("%s", action.c_str());
        ImGui::SameLine(260.0f);
        ImGui::TextDisabled("%s", combo.toString().c_str());
    }
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::End();
}
