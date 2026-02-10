#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../AnimationUtils.h"

static bool renderToggleButton(const char* label, bool value) {
    ImVec4 active = ImVec4(0.25f, 0.55f, 0.85f, 0.9f);
    ImVec4 inactive = ImVec4(0.20f, 0.20f, 0.20f, 0.9f);
    ImGui::PushStyleColor(ImGuiCol_Button, value ? active : inactive);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, value ? active : inactive);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, value ? active : inactive);
    bool pressed = ImGui::Button(label);
    ImGui::PopStyleColor(3);
    return pressed;
}

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
    state.refreshSearchMatches();

    bool hasSelection = state.active() && state.active()->widget.hasSelectionRange();
    if (state.search.findInSelection && !hasSelection) {
        ImGui::TextDisabled("Find in selection: no selection (searching full file)");
    }
    ImGui::SetNextItemWidth(300);
    if (ImGui::InputText("Find", state.search.findBuf, sizeof(state.search.findBuf),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        state.search.lastFindPos = 0;
        state.doFindNext(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Find Next")) state.doFindNext(false);
    ImGui::SameLine();
    if (ImGui::Button("Find Prev")) state.doFindNext(true);

    if (state.search.showReplace) {
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("Replace", state.search.replaceBuf, sizeof(state.search.replaceBuf));
        ImGui::SameLine();
        if (ImGui::Button("Replace")) state.doReplaceCurrent();
        ImGui::SameLine();
        if (ImGui::Button("Replace All")) state.doReplaceAll();
    }

    ImGui::Separator();
    if (renderToggleButton("Match Case", state.search.matchCase)) {
        state.search.matchCase = !state.search.matchCase;
    }
    ImGui::SameLine();
    if (renderToggleButton("Whole Word", state.search.wholeWord)) {
        state.search.wholeWord = !state.search.wholeWord;
    }
    ImGui::SameLine();
    if (renderToggleButton("Regex", state.search.useRegex)) {
        state.search.useRegex = !state.search.useRegex;
    }
    ImGui::SameLine();
    if (renderToggleButton("In Selection", state.search.findInSelection)) {
        state.search.findInSelection = !state.search.findInSelection;
    }

    if (!state.search.findHistory.empty()) {
        ImGui::SameLine();
        if (ImGui::BeginCombo("History", "")) {
            for (const auto& item : state.search.findHistory) {
                if (ImGui::Selectable(item.c_str())) {
                    std::snprintf(state.search.findBuf, sizeof(state.search.findBuf), "%s", item.c_str());
                    state.search.lastFindPos = 0;
                }
            }
            ImGui::EndCombo();
        }
    }

    if (!state.search.matches.empty()) {
        int current = state.search.currentMatchIndex >= 0
            ? state.search.currentMatchIndex + 1
            : 0;
        ImGui::TextDisabled("%d of %d matches", current, (int)state.search.matches.size());
        if (state.search.currentMatchIndex >= 0 &&
            state.search.currentMatchIndex < (int)state.search.matches.size()) {
            const auto& m = state.search.matches[state.search.currentMatchIndex];
            ImGui::SameLine();
            ImGui::TextDisabled("Ln %d, Col %d", m.line + 1, m.col + 1);
        }
    } else if (state.search.findBuf[0] != '\0') {
        ImGui::TextDisabled("0 matches");
    }

    if (state.search.useRegex && state.search.findBuf[0] != '\0' && state.active()) {
        std::vector<std::string> groups;
        int rangeStart = 0;
        int rangeEnd = (int)state.active()->editBuf.size();
        if (state.search.findInSelection && hasSelection) {
            state.active()->widget.getSelectionRange(rangeStart, rangeEnd);
        }
        bool ok = SearchUtils::regexGroupsForFirstMatch(state.active()->editBuf,
                                                        state.search.findBuf,
                                                        state.currentSearchOptions(),
                                                        rangeStart,
                                                        rangeEnd,
                                                        groups);
        if (!ok) {
            ImGui::TextDisabled("Regex preview: no match");
        } else if (!groups.empty()) {
            ImGui::TextDisabled("Regex groups:");
            for (size_t i = 0; i < groups.size(); ++i) {
                ImGui::Text("  $%d = %s", (int)i + 1, groups[i].c_str());
            }
        }
    }

    if (state.search.showReplace && !state.search.matches.empty()) {
        int idx = state.search.currentMatchIndex;
        if (idx < 0 || idx >= (int)state.search.matches.size()) idx = 0;
        const auto& m = state.search.matches[idx];
        std::string replacement = SearchUtils::applyReplacement(m.matchText,
                                                                state.search.findBuf,
                                                                state.search.replaceBuf,
                                                                state.currentSearchOptions());
        std::string preview = m.lineText;
        if (m.col >= 0 && m.col <= (int)preview.size()) {
            int replaceLen = m.end - m.start;
            if (m.col + replaceLen <= (int)preview.size()) {
                preview.replace(m.col, replaceLen, replacement);
            }
        }
        ImGui::Separator();
        ImGui::TextDisabled("Replace preview:");
        ImGui::Text("Before: %s", m.lineText.c_str());
        ImGui::Text("After : %s", preview.c_str());
    }
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
                        else state.doOpen(fileRes.path, state.defaultBufferMode());
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
