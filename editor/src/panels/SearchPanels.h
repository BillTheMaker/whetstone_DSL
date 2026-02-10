#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"

static void renderFindReplaceBar(EditorState& state) {
    if (!state.search.showFind) return;
    ImGui::Begin("Find & Replace", &state.search.showFind, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushFont(state.uiFont);
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
    ImGui::End();
}

static void renderProjectSearchPanel(EditorState& state) {
    if (!state.search.showProjectSearch) return;
    ImGui::Begin("Search", &state.search.showProjectSearch);
    ImGui::PushFont(state.uiFont);
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
    ImGui::End();
}
