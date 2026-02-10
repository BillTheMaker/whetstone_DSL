#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include <filesystem>
#include <unordered_set>

static int buildCommandContextMask(const EditorState& state) {
    int mask = CommandContext_Editor;
    if (state.search.showProjectSearch || state.search.showFind) mask |= CommandContext_Search;
    if (state.ui.showSettingsPanel || state.ui.showLspSettings) mask |= CommandContext_Settings;
    if (state.ui.showHelpPanel || state.ui.showShortcutReference) mask |= CommandContext_Help;
    if (state.build.showTerminalPanel) mask |= CommandContext_Terminal;
    return mask;
}

static void collectFilePaths(const FileNode& node, std::vector<std::string>& out) {
    if (!node.isDir) {
        out.push_back(node.path);
        return;
    }
    for (const auto& child : node.children) {
        collectFilePaths(child, out);
    }
}

static std::string makeRelativePath(const std::string& path, const std::string& root) {
    if (root.empty()) return path;
    std::error_code ec;
    auto rel = std::filesystem::relative(path, root, ec);
    if (ec) return path;
    return rel.generic_string();
}

static void drawHighlightedText(ImDrawList* drawList,
                                const std::string& text,
                                const std::vector<int>& indices,
                                ImVec2 pos,
                                ImU32 normalColor,
                                ImU32 highlightColor) {
    std::vector<unsigned char> highlight(text.size(), 0);
    for (int idx : indices) {
        if (idx >= 0 && idx < (int)highlight.size()) highlight[idx] = 1;
    }
    ImFont* font = ImGui::GetFont();
    float fontSize = ImGui::GetFontSize();
    char buf[2] = {0, 0};
    for (size_t i = 0; i < text.size(); ++i) {
        buf[0] = text[i];
        drawList->AddText(font, fontSize, pos,
                          highlight[i] ? highlightColor : normalColor, buf);
        pos.x += ImGui::CalcTextSize(buf).x;
    }
}

static void renderGoToLineDialog(EditorState& state) {
    if (state.search.showGoToLine) {
        ImGui::OpenPopup("GoToLine");
    }
    if (ImGui::BeginPopupModal("GoToLine", &state.search.showGoToLine, ImGuiWindowFlags_AlwaysAutoResize)) {
        int totalLines = state.active() ? countLines(state.active()->editBuf) : 0;
        ImGui::Text("Enter line or :line:col");
        ImGui::TextDisabled("Total lines: %d", totalLines);
        ImGui::SetNextItemWidth(240);
        bool submit = ImGui::InputText("##gotoLineInput", state.search.goToLineBuf,
                                       sizeof(state.search.goToLineBuf),
                                       ImGuiInputTextFlags_EnterReturnsTrue);
        if (ImGui::Button("Go")) submit = true;
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            state.search.showGoToLine = false;
            state.search.goToLineError = false;
            ImGui::CloseCurrentPopup();
        }
        if (submit) {
            int line = 0;
            int col = 0;
            if (parseLineColInput(state.search.goToLineBuf, line, col)) {
                if (totalLines > 0) line = std::max(1, std::min(line, totalLines));
                col = std::max(1, col);
                if (state.active()) {
                    state.jumpTo(state.active(), line - 1, col - 1);
                }
                state.search.showGoToLine = false;
                state.search.goToLineError = false;
                ImGui::CloseCurrentPopup();
            } else {
                state.search.goToLineError = true;
            }
        }
        if (state.search.goToLineError) {
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Invalid format.");
        }
        ImGui::EndPopup();
    }
}

static void renderLspSettingsPanel(EditorState& state) {
    if (!state.ui.showLspSettings) return;
    ImGui::Begin("LSP Servers", &state.ui.showLspSettings);
    ImGui::PushFont(state.uiFont);
    ImGui::TextUnformatted("Emacs Config");
    ImGui::SetNextItemWidth(320);
    InputTextStr("Config Path", &state.settings.getEmacsConfigPathMutable(), ImGuiInputTextFlags_None);
    ImGui::Separator();
    if (ImGui::Button("Auto-Detect")) {
        state.settings.autoDetect();
    }
    ImGui::Separator();
    for (auto& cfg : state.settings.getLSPServersMutable()) {
        ImGui::PushID(cfg.language.c_str());
        ImGui::Checkbox("Enabled", &cfg.enabled);
        ImGui::SameLine();
        ImGui::Text("%s", cfg.language.c_str());
        ImGui::SetNextItemWidth(320);
        InputTextStr("Path", &cfg.path);
        ImGui::SetNextItemWidth(320);
        if (InputTextStr("Args", &cfg.argsLine)) {
            state.settings.syncArgs(cfg);
        }
        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::PopFont();
    ImGui::End();
}

static void renderRefactorPopup(EditorState& state) {
    if (state.showRefactorPopup) {
        ImGui::OpenPopup("RefactorPopup");
    }
    if (ImGui::BeginPopupModal("RefactorPopup", &state.showRefactorPopup,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        const char* title = "";
        if (state.refactorAction == 1) title = "Rename Variable";
        else if (state.refactorAction == 2) title = "Extract Function";
        else if (state.refactorAction == 3) title = "Inline Variable";
        ImGui::TextUnformatted(title);
        ImGui::Separator();

        if (state.refactorAction == 1) {
            ImGui::InputText("Old Name", state.refactorNameA, sizeof(state.refactorNameA));
            ImGui::InputText("New Name", state.refactorNameB, sizeof(state.refactorNameB));
        } else if (state.refactorAction == 2) {
            ImGui::InputText("New Function Name", state.refactorNameA, sizeof(state.refactorNameA));
            ImGui::TextDisabled("Extracts the first statement of the first function.");
        } else if (state.refactorAction == 3) {
            ImGui::InputText("Variable Name", state.refactorNameA, sizeof(state.refactorNameA));
            ImGui::TextDisabled("Removes the first matching variable declaration.");
        }

        if (!state.refactorError.empty()) {
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f),
                               "%s", state.refactorError.c_str());
        }

        bool canRun = state.isStructured() && state.active() && !state.active()->readOnly;
        if (!canRun) ImGui::BeginDisabled();
        if (ImGui::Button("Preview")) {
            state.refactorError.clear();
            Module* ast = state.activeAST();
            RefactorPlan plan;
            if (state.refactorAction == 1) {
                plan = buildRenameVariablePlan(ast, state.refactorNameA, state.refactorNameB);
            } else if (state.refactorAction == 2) {
                plan = buildExtractFunctionPlan(ast, state.refactorNameA);
            } else if (state.refactorAction == 3) {
                plan = buildInlineVariablePlan(ast, state.refactorNameA);
            }
            if (!plan.success) {
                state.refactorError = plan.error;
            } else {
                auto previewAst = cloneModule(ast);
                BatchMutationAPI batch;
                batch.setRoot(previewAst.get());
                auto res = batch.applySequence(plan.mutations);
                if (!res.success) {
                    state.refactorError = res.error;
                } else {
                    std::string beforeText = state.active()->editBuf;
                    std::string afterText =
                        generateForLanguage(previewAst.get(), state.active()->language);
                    state.openDiff(beforeText, afterText, true, 0, {}, plan.mutations);
                    state.showRefactorPopup = false;
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply")) {
            state.refactorError.clear();
            Module* ast = state.mutationAST();
            RefactorPlan plan;
            if (state.refactorAction == 1) {
                plan = buildRenameVariablePlan(ast, state.refactorNameA, state.refactorNameB);
            } else if (state.refactorAction == 2) {
                plan = buildExtractFunctionPlan(ast, state.refactorNameA);
            } else if (state.refactorAction == 3) {
                plan = buildInlineVariablePlan(ast, state.refactorNameA);
            }
            if (!plan.success) {
                state.refactorError = plan.error;
            } else {
                BatchMutationAPI batch;
                batch.setRoot(ast);
                auto res = batch.applySequence(plan.mutations);
                if (!res.success) {
                    state.refactorError = res.error;
                } else {
                    state.notify(NotificationLevel::Success,
                                 "Refactor applied (" +
                                 std::to_string(res.appliedCount) + " mutations).");
                    state.applyOrchestratorToActive();
                    state.showRefactorPopup = false;
                }
            }
        }
        if (!canRun) ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            state.showRefactorPopup = false;
        }

        ImGui::EndPopup();
    }
}

static void renderCommandPalette(EditorState& state) {
    if (!state.showCommandPalette) return;
    ImGui::Begin("Command Palette", &state.showCommandPalette,
                 ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetNextItemWidth(520.0f);
    if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
    }
    const bool hintCommandMode = (state.commandQuery[0] == '>');
    const char* queryHint = hintCommandMode
        ? "> Type a command"
        : "Type to search files (prefix '>' for commands)";
    bool queryChanged = ImGui::InputTextWithHint("##cmdQuery",
                                                 queryHint,
                                                 state.commandQuery,
                                                 sizeof(state.commandQuery),
                                                 ImGuiInputTextFlags_EnterReturnsTrue);
    if (queryChanged) {
        state.commandSelected = 0;
    }

    const bool dismiss = ImGui::IsKeyPressed(ImGuiKey_Escape);
    if (dismiss) state.showCommandPalette = false;

    std::string rawQuery = state.commandQuery;
    bool commandMode = false;
    std::string query = rawQuery;
    if (!query.empty() && query[0] == '>') {
        commandMode = true;
        query.erase(0, 1);
        if (!query.empty() && query[0] == ' ') query.erase(0, 1);
    }

    if (commandMode) {
        const int contextMask = buildCommandContextMask(state);
        const bool strictContext = query.empty();
        auto results = state.commandPalette.search(query, contextMask, strictContext);
        std::vector<CommandEntry> recent =
            query.empty() ? state.commandPalette.recent(5, contextMask) : std::vector<CommandEntry>();
        std::vector<CommandMatch> recentMatches;
        recentMatches.reserve(recent.size());
        std::unordered_set<std::string> recentIds;
        for (const auto& entry : recent) {
            recentIds.insert(entry.id);
            CommandMatch match;
            match.entry = entry;
            recentMatches.push_back(std::move(match));
        }

        struct CategoryGroup {
            std::string label;
            std::vector<CommandMatch> items;
            int bestScore = 0;
        };

        std::unordered_map<std::string, CategoryGroup> groupsByName;
        for (const auto& match : results) {
            if (query.empty() && recentIds.count(match.entry.id)) continue;
            std::string category = match.entry.category.empty() ? "Other" : match.entry.category;
            auto& group = groupsByName[category];
            group.label = category;
            group.items.push_back(match);
            group.bestScore = std::max(group.bestScore, match.score);
        }

        std::vector<CategoryGroup> groups;
        groups.reserve(groupsByName.size());
        for (auto& [_, group] : groupsByName) {
            std::sort(group.items.begin(), group.items.end(),
                      [](const CommandMatch& a, const CommandMatch& b) {
                          if (a.score != b.score) return a.score > b.score;
                          return a.entry.label < b.entry.label;
                      });
            groups.push_back(std::move(group));
        }
        std::sort(groups.begin(), groups.end(),
                  [](const CategoryGroup& a, const CategoryGroup& b) {
                      if (a.bestScore != b.bestScore) return a.bestScore > b.bestScore;
                      return a.label < b.label;
                  });

        int totalItems = (int)recentMatches.size();
        for (const auto& group : groups) totalItems += (int)group.items.size();
        int maxIndex = std::max(0, totalItems - 1);
        state.commandSelected = std::min(state.commandSelected, maxIndex);
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            state.commandSelected = std::min(state.commandSelected + 1, maxIndex);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            state.commandSelected = std::max(state.commandSelected - 1, 0);
        }

        const ImU32 normalColor = ImGui::GetColorU32(ImGuiCol_Text);
        const ImU32 disabledColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
        const ImU32 highlightColor = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
        int displayIndex = 0;
        const CommandMatch* selectedMatch = nullptr;
        bool accept = ImGui::IsKeyPressed(ImGuiKey_Enter);

        if (!recentMatches.empty()) {
            ImGui::TextDisabled("Recent");
            ImGui::Separator();
            for (const auto& match : recentMatches) {
                bool selected = (displayIndex == state.commandSelected);
                std::string rowId = "##cmd_recent_" + std::to_string(displayIndex);
                if (ImGui::Selectable(rowId.c_str(), selected,
                                      ImGuiSelectableFlags_SpanAvailWidth,
                                      ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing()))) {
                    state.commandSelected = displayIndex;
                    accept = true;
                }
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImVec2 textPos(min.x + ImGui::GetStyle().FramePadding.x,
                               min.y + ImGui::GetStyle().FramePadding.y);
                drawHighlightedText(ImGui::GetWindowDrawList(),
                                    match.entry.label,
                                    match.matchIndices,
                                    textPos,
                                    normalColor,
                                    highlightColor);
                if (!match.entry.shortcut.empty()) {
                    ImVec2 size = ImGui::CalcTextSize(match.entry.shortcut.c_str());
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(max.x - ImGui::GetStyle().FramePadding.x - size.x, textPos.y),
                        disabledColor,
                        match.entry.shortcut.c_str());
                }
                if (selected) selectedMatch = &match;
                ++displayIndex;
            }
        }

        for (const auto& group : groups) {
            if (group.items.empty()) continue;
            if (!recentMatches.empty() || !group.label.empty()) {
                if (displayIndex > 0) ImGui::Separator();
                ImGui::TextDisabled("%s", group.label.c_str());
            }
            for (const auto& match : group.items) {
                bool selected = (displayIndex == state.commandSelected);
                std::string rowId = "##cmd_" + std::to_string(displayIndex);
                if (ImGui::Selectable(rowId.c_str(), selected,
                                      ImGuiSelectableFlags_SpanAvailWidth,
                                      ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing()))) {
                    state.commandSelected = displayIndex;
                    accept = true;
                }
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImVec2 textPos(min.x + ImGui::GetStyle().FramePadding.x,
                               min.y + ImGui::GetStyle().FramePadding.y);
                drawHighlightedText(ImGui::GetWindowDrawList(),
                                    match.entry.label,
                                    match.matchIndices,
                                    textPos,
                                    normalColor,
                                    highlightColor);
                if (!match.entry.shortcut.empty()) {
                    ImVec2 size = ImGui::CalcTextSize(match.entry.shortcut.c_str());
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(max.x - ImGui::GetStyle().FramePadding.x - size.x, textPos.y),
                        disabledColor,
                        match.entry.shortcut.c_str());
                }
                if (selected) selectedMatch = &match;
                ++displayIndex;
            }
        }

        if (totalItems == 0) {
            ImGui::TextDisabled("No matching commands.");
        }

        if (selectedMatch && selectedMatch->entry.inlineInput) {
            if (selectedMatch->entry.id != state.commandInlineCommandId) {
                state.commandInlineCommandId = selectedMatch->entry.id;
                state.commandInlineValue[0] = '\0';
            }
            ImGui::Separator();
            ImGui::TextDisabled("Input");
            ImGui::SetNextItemWidth(300.0f);
            bool inlineSubmit = ImGui::InputTextWithHint("##cmdInlineInput",
                                                         selectedMatch->entry.inputHint.c_str(),
                                                         state.commandInlineValue,
                                                         sizeof(state.commandInlineValue),
                                                         ImGuiInputTextFlags_EnterReturnsTrue);
            if (inlineSubmit) {
                state.executeCommand(selectedMatch->entry.id,
                                     std::string(state.commandInlineValue));
                state.showCommandPalette = false;
            } else if (accept) {
                ImGui::SetKeyboardFocusHere(-1);
            }
        } else if (accept && totalItems > 0 && state.commandSelected >= 0 &&
                   state.commandSelected < totalItems && selectedMatch) {
            state.executeCommand(selectedMatch->entry.id);
            state.showCommandPalette = false;
        }
    } else {
        struct FileMatch {
            std::string path;
            std::string display;
            int score = 0;
            std::vector<int> matchIndices;
        };

        static std::string cachedRoot;
        static std::vector<std::string> cachedFiles;
        if (state.workspaceRoot.empty()) {
            cachedFiles.clear();
            cachedRoot.clear();
        } else if (cachedRoot != state.workspaceRoot || state.fileTreeDirty) {
            state.refreshFileTree();
            cachedFiles.clear();
            if (!state.fileTreeRoot.path.empty()) {
                collectFilePaths(state.fileTreeRoot, cachedFiles);
            }
            cachedRoot = state.workspaceRoot;
        }

        std::vector<FileMatch> fileMatches;
        if (!query.empty()) {
            fileMatches.reserve(cachedFiles.size());
            for (const auto& path : cachedFiles) {
                FileMatch match;
                match.path = path;
                match.display = makeRelativePath(path, state.workspaceRoot);
                int score = 0;
                if (!CommandPalette::fuzzyMatch(match.display, query, score, match.matchIndices)) {
                    continue;
                }
                match.score = score;
                fileMatches.push_back(std::move(match));
            }
            std::sort(fileMatches.begin(), fileMatches.end(),
                      [](const FileMatch& a, const FileMatch& b) {
                          if (a.score != b.score) return a.score > b.score;
                          return a.display < b.display;
                      });
            if (fileMatches.size() > 60) fileMatches.resize(60);
        }

        int maxIndex = std::max(0, (int)fileMatches.size() - 1);
        state.commandSelected = std::min(state.commandSelected, maxIndex);
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            state.commandSelected = std::min(state.commandSelected + 1, maxIndex);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            state.commandSelected = std::max(state.commandSelected - 1, 0);
        }
        bool accept = ImGui::IsKeyPressed(ImGuiKey_Enter);

        const ImU32 normalColor = ImGui::GetColorU32(ImGuiCol_Text);
        const ImU32 disabledColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
        const ImU32 highlightColor = ImGui::GetColorU32(ImGuiCol_PlotHistogram);

        if (state.workspaceRoot.empty()) {
            ImGui::TextDisabled("Open a folder to search files.");
        } else if (query.empty()) {
            ImGui::TextDisabled("Start typing to search files.");
        } else if (fileMatches.empty()) {
            ImGui::TextDisabled("No matching files.");
        }

        for (int i = 0; i < (int)fileMatches.size(); ++i) {
            const auto& match = fileMatches[i];
            bool selected = (i == state.commandSelected);
            std::string rowId = "##file_" + std::to_string(i);
            if (ImGui::Selectable(rowId.c_str(), selected,
                                  ImGuiSelectableFlags_SpanAvailWidth,
                                  ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing()))) {
                state.commandSelected = i;
                accept = true;
            }
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImVec2 textPos(min.x + ImGui::GetStyle().FramePadding.x,
                           min.y + ImGui::GetStyle().FramePadding.y);
            drawHighlightedText(ImGui::GetWindowDrawList(),
                                match.display,
                                match.matchIndices,
                                textPos,
                                normalColor,
                                highlightColor);
            std::string folder = std::filesystem::path(match.path).parent_path().string();
            if (!folder.empty()) {
                ImVec2 size = ImGui::CalcTextSize(folder.c_str());
                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(max.x - ImGui::GetStyle().FramePadding.x - size.x, textPos.y),
                    disabledColor,
                    folder.c_str());
            }
        }

        if (accept && !fileMatches.empty()) {
            state.doOpen(fileMatches[state.commandSelected].path);
            state.showCommandPalette = false;
        }
    }
    ImGui::End();
}
