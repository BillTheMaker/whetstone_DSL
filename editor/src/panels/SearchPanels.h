#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../AnimationUtils.h"

static void renderFindReplaceBar(EditorState& state) {
    const double now = ImGui::GetTime();
    float alpha = 1.0f;
    float offset = 0.0f;
    bool render = AnimationUtils::panelTransition("FindReplacePanel",
                                                  state.search.showFind,
                                                  now,
                                                  state.settings.getReduceMotion(),
                                                  0.14f,
                                                  12.0f,
                                                  alpha,
                                                  offset);
    if (!render) return;
    bool open = state.search.showFind;
    ImGui::SetNextWindowBgAlpha(alpha);
    ImGui::Begin("Find & Replace", &open, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
    ImGui::PushFont(state.uiFont);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::SetNextItemWidth(300);
    if (ImGui::InputText("Find", state.search.findBuf, sizeof(state.search.findBuf),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        state.doFind();
    }
    ImGui::SameLine();
    if (ImGui::Button("Find Next")) state.doFind();

    ImGui::SetNextItemWidth(300);
    ImGui::InputText("Replace", state.search.replaceBuf, sizeof(state.search.replaceBuf));
    ImGui::SameLine();
    if (ImGui::Button("Replace All")) state.doReplaceAll();
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::End();
    if (!open) {
        state.search.showFind = false;
    }
}

static void renderProjectSearchPanel(EditorState& state) {
    const double now = ImGui::GetTime();
    float alpha = 1.0f;
    float offset = 0.0f;
    bool render = AnimationUtils::panelTransition("ProjectSearchPanel",
                                                  state.search.showProjectSearch,
                                                  now,
                                                  state.settings.getReduceMotion(),
                                                  0.16f,
                                                  16.0f,
                                                  alpha,
                                                  offset);
    if (!render) return;
    bool open = state.search.showProjectSearch;
    ImGui::SetNextWindowBgAlpha(alpha);
    ImGui::Begin("Search", &open);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
    ImGui::PushFont(state.uiFont);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    bool doSearch = false;
    ImGui::SetNextItemWidth(420);
    if (ImGui::InputText("Query", state.search.searchQuery, sizeof(state.search.searchQuery),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        doSearch = true;
    }
    ImGui::SetNextItemWidth(420);
    ImGui::InputText("Include (glob)", state.search.searchInclude, sizeof(state.search.searchInclude));
    ImGui::SetNextItemWidth(420);
    ImGui::InputText("Exclude (glob)", state.search.searchExclude, sizeof(state.search.searchExclude));
    ImGui::Checkbox("Regex", &state.search.searchUseRegex);
    ImGui::SameLine();
    if (ImGui::Button("Search")) doSearch = true;
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        state.search.searchQuery[0] = '\0';
        state.search.searchResults.clear();
    }

    if (doSearch) {
        state.search.searchResults = state.search.projectSearch.search(
            state.search.searchQuery,
            state.search.searchInclude,
            state.search.searchExclude,
            state.search.searchUseRegex);
    }

    ImGui::Separator();
    ImGui::BeginChild("##searchResults", ImVec2(0, 0), false);
    if (state.search.searchResults.empty()) {
        ImGui::TextDisabled("(no results)");
    } else {
        for (const auto& fileRes : state.search.searchResults) {
            std::string label = fileRes.path + " (" + std::to_string(fileRes.matches.size()) + ")";
            if (ImGui::TreeNode(label.c_str())) {
                for (const auto& match : fileRes.matches) {
                    std::string lineLabel = std::to_string(match.line + 1) + ":" +
                        std::to_string(match.col + 1) + "  " + match.lineText;
                    if (ImGui::Selectable(lineLabel.c_str())) {
                        if (state.buffers.hasBuffer(fileRes.path)) state.switchToBuffer(fileRes.path);
                        else state.doOpen(fileRes.path);
                        state.jumpTo(state.active(), match.line, match.col);
                    }
                }
                ImGui::TreePop();
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::End();
    if (!open) {
        state.search.showProjectSearch = false;
    }
}
