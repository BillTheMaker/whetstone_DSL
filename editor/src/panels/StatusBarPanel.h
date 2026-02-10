#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"

static void renderStatusBar(EditorState& state) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags sbFlags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings;
    float sbHeight = ImGui::GetFrameHeight() + 2;
    ImVec2 sbPos(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - sbHeight);
    ImVec2 sbSize(viewport->WorkSize.x, sbHeight);
    ImGui::SetNextWindowPos(sbPos);
    ImGui::SetNextWindowSize(sbSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.00f, 0.47f, 0.84f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));
    ImGui::Begin("##StatusBar", nullptr, sbFlags);
    ImGui::PushFont(state.uiFont);

    if (state.active())
        ImGui::Text("Ln %d, Col %d", state.active()->cursorLine, state.active()->cursorCol);
    else
        ImGui::Text("Ln -, Col -");
    ImGui::SameLine(0, 30);

    if (state.active())
        ImGui::Text("%s", state.active()->language.c_str());
    else
        ImGui::Text("-");
    ImGui::SameLine(0, 30);

    if (state.active()) {
        const char* modeLabel =
            state.active()->bufferMode == BufferManager::BufferMode::Text ? "Text" : "Structured";
        ImGui::Text("Mode: %s", modeLabel);
    } else {
        ImGui::Text("Mode: -");
    }
    ImGui::SameLine(0, 30);

    ImGui::Text("Keys: %s", KeybindingManager::profileName(state.keys.getProfile()));
    ImGui::SameLine(0, 30);
    if (state.ui.layoutPreset == LayoutPreset::Emacs && !state.emacsState.emacsKeys.modeLine.empty()) {
        ImGui::Text("Mode: %s", state.emacsState.emacsKeys.modeLine.c_str());
        ImGui::SameLine(0, 30);
    }

    ImGui::Text("Zoom: %d%%", zoomPercent(state.settings.getFontSize(), state.baseFontSize));
    ImGui::SameLine(0, 30);

    ImGui::Text("Undo: %d", state.active() ? state.active()->undoDepth : 0);
    ImGui::SameLine(0, 30);

    if (state.build.runInProgress) {
        ImGui::Text("Run: Running...");
    } else if (state.build.hasRunResult) {
        ImGui::Text("Run: Exit %d", state.build.lastRunExitCode);
    } else {
        ImGui::Text("Run: -");
    }
    ImGui::SameLine(0, 30);

    if (state.active() && state.active()->modified)
        ImGui::Text("Modified");
    else
        ImGui::Text("Saved");

    ImGui::SameLine(0, 30);
    ImGui::Text("UTF-8");

    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
