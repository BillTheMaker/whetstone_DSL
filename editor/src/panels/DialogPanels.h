#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"

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
                    state.outputLog += "Refactor applied (" +
                                       std::to_string(res.appliedCount) + " mutations).\n";
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
    ImGui::SetNextItemWidth(420.0f);
    if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
    }
    if (ImGui::InputText("##cmdQuery", state.commandQuery, sizeof(state.commandQuery))) {
        state.commandSelected = 0;
    }

    auto results = state.commandPalette.search(state.commandQuery);
    int maxIndex = std::max(0, (int)results.size() - 1);
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        state.commandSelected = std::min(state.commandSelected + 1, maxIndex);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        state.commandSelected = std::max(state.commandSelected - 1, 0);
    }
    bool accept = ImGui::IsKeyPressed(ImGuiKey_Enter);
    bool dismiss = ImGui::IsKeyPressed(ImGuiKey_Escape);

    if (dismiss) {
        state.showCommandPalette = false;
    }

    for (int i = 0; i < (int)results.size(); ++i) {
        const auto& cmd = results[i];
        bool selected = (i == state.commandSelected);
        std::string label = cmd.label;
        if (!cmd.shortcut.empty()) label += "  [" + cmd.shortcut + "]";
        if (ImGui::Selectable(label.c_str(), selected)) {
            state.commandSelected = i;
            accept = true;
        }
    }
    if (accept && !results.empty()) {
        state.executeCommand(results[state.commandSelected].id);
        state.showCommandPalette = false;
    }
    ImGui::End();
}
