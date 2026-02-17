#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../ThemeEngine.h"

static void renderMenuBar(EditorState& state) {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", state.keys.getBinding("file.new").toString().c_str()))
        {
            std::string lang = state.active() ? state.active()->language : "python";
            state.createBuffer(state.makeUntitledName(), "", lang, state.defaultBufferMode());
        }
        if (ImGui::MenuItem("Open...", state.keys.getBinding("file.open").toString().c_str()))
        {
            auto path = FileDialog::openFile({"Open File", {"*.py","*.cpp","*.h","*.el","*.js","*.ts","*.java","*.rs","*.go","*.org"}, state.lastDialogPath});
            if (!path.empty()) {
                state.lastDialogPath = path;
                state.doOpen(path, state.defaultBufferMode());
            }
        }
        if (ImGui::MenuItem("Open Project...")) {
            auto path = FileDialog::openFile({"Open Project", {"*.whetstone"}, state.lastDialogPath});
            if (!path.empty()) {
                if (!state.loadProject(path)) {
                    state.notify(NotificationLevel::Error,
                                 "Failed to open project: " + path);
                } else {
                    state.lastDialogPath = path;
                }
            }
        }
        if (ImGui::MenuItem("Open Folder...")) {
            auto path = FileDialog::openFolder({"Open Folder", state.workspaceRoot});
            if (!path.empty()) {
                state.workspaceRoot = path;
                state.fileTreeDirty = true;
                state.search.projectSearch.setRoot(state.workspaceRoot);
                state.lastDialogPath = path;
                state.refreshBuildSystem();
            }
        }
        if (ImGui::MenuItem("Save", state.keys.getBinding("file.save").toString().c_str()))
        {
            if (!state.active()) {
                // no-op
            } else if (state.active()->path.rfind("(untitled", 0) == 0) {
                auto path = FileDialog::saveFile({"Save File", {"*.*"}, state.lastDialogPath});
                if (!path.empty()) {
                    state.active()->path = path;
                    state.lastDialogPath = path;
                    state.doSave();
                }
            } else {
                state.doSave();
            }
        }
        if (ImGui::MenuItem("Save Project...")) {
            auto path = FileDialog::saveFile({"Save Project", {"*.whetstone"}, state.lastDialogPath});
            if (!path.empty()) {
                if (!state.saveProject(path)) {
                    state.notify(NotificationLevel::Error,
                                 "Failed to save project: " + path);
                } else {
                    state.lastDialogPath = path;
                }
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) state.exitRequested = true;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", state.keys.getBinding("edit.undo").toString().c_str(),
                            false, state.active() ? state.active()->orchestrator.getUndoDepth() > 0 : false))
            state.doUndo();
        if (ImGui::MenuItem("Redo", state.keys.getBinding("edit.redo").toString().c_str(),
                            false, state.active() ? state.active()->orchestrator.getRedoDepth() > 0 : false))
            state.doRedo();
        ImGui::Separator();
        if (ImGui::MenuItem("Find/Replace", state.keys.getBinding("search.find").toString().c_str())) {
            state.search.showFind = true;
            state.search.showReplace = true;
        }
        if (ImGui::MenuItem("Find in Files", state.keys.getBinding("search.findInFiles").toString().c_str()))
            state.search.showProjectSearch = !state.search.showProjectSearch;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Show Whitespace", nullptr, &state.ui.showWhitespace);
        ImGui::MenuItem("Show Minimap", nullptr, &state.ui.showMinimap);
        ImGui::MenuItem("Inline Completion Helper", nullptr, &state.ui.showCompletionHelper);
        ImGui::MenuItem("Show Annotations", nullptr, &state.ui.showAnnotations);
        ImGui::MenuItem("Show Outline", nullptr, &state.ui.showOutline);
        ImGui::MenuItem("Terminal", state.keys.getBinding("view.toggleTerminal").toString().c_str(),
                        &state.build.showTerminalPanel);
        ImGui::MenuItem("Dependencies", nullptr, &state.library.showDependencyPanel);
        ImGui::MenuItem("Libraries", nullptr, &state.library.showLibraryBrowserPanel);
        ImGui::MenuItem("Compose", nullptr, &state.library.showCompositionPanel);
        ImGui::MenuItem("Memory Strategies", nullptr, &state.ui.showMemoryStrategies);
        ImGui::MenuItem("Emacs Packages", nullptr, &state.emacsState.showEmacsPackagesPanel);
        ImGui::MenuItem("Emacs Bridge", nullptr, &state.emacsState.showEmacsBridgePanel);
        ImGui::MenuItem("Settings", nullptr, &state.ui.showSettingsPanel);
        ImGui::MenuItem("Keyboard Shortcuts", nullptr, &state.ui.showShortcutReference);
        ImGui::MenuItem("LSP Servers...", nullptr, &state.ui.showLspSettings);
        if (state.active()) {
            bool textMode = state.active()->bufferMode == BufferManager::BufferMode::Text;
            if (ImGui::MenuItem("Text-Editor Mode", nullptr, textMode)) {
                state.active()->bufferMode = textMode ?
                    BufferManager::BufferMode::Structured :
                    BufferManager::BufferMode::Text;
                state.buffers.setBufferMode(state.active()->path, state.active()->bufferMode);
                if (state.active()->bufferMode == BufferManager::BufferMode::Text) {
                    state.suggestions.clear();
                    state.whetstoneDiagnostics.clear();
                    state.analysisPending = false;
                } else {
                    state.onTextChanged();
                }
                if (state.active()->path.rfind("(untitled", 0) != 0) {
                    state.welcome.addRecentFile(state.active()->path,
                                                state.active()->language,
                                                bufferModeToString(state.active()->bufferMode));
                    state.saveRecentFiles();
                }
            }
        }
        if (ImGui::BeginMenu("Layout")) {
            if (ImGui::MenuItem("VSCode", nullptr, state.ui.layoutPreset == LayoutPreset::VSCode))
                state.ui.layoutPreset = LayoutPreset::VSCode;
            if (ImGui::MenuItem("Emacs", nullptr, state.ui.layoutPreset == LayoutPreset::Emacs))
                state.ui.layoutPreset = LayoutPreset::Emacs;
            if (ImGui::MenuItem("JetBrains", nullptr, state.ui.layoutPreset == LayoutPreset::JetBrains))
                state.ui.layoutPreset = LayoutPreset::JetBrains;
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Layout")) {
                state.ui.requestLayoutReset = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Navigate")) {
        if (ImGui::MenuItem("Go to Line...", state.keys.getBinding("nav.goToLine").toString().c_str())) {
            state.search.showGoToLine = true;
            state.search.goToLineBuf[0] = '\0';
            state.search.goToLineError = false;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Build")) {
        if (ImGui::MenuItem("Run", state.keys.getBinding("build.run").toString().c_str())) {
            state.runActiveFile(false);
        }
        if (ImGui::MenuItem("Build", state.keys.getBinding("build.build").toString().c_str())) {
            state.runActiveFile(true);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Language")) {
        if (ImGui::MenuItem("Python", nullptr, state.active() && state.active()->language == "python"))
            state.setLanguage("python");
        if (ImGui::MenuItem("C++", nullptr, state.active() && state.active()->language == "cpp"))
            state.setLanguage("cpp");
        if (ImGui::MenuItem("Elisp", nullptr, state.active() && state.active()->language == "elisp"))
            state.setLanguage("elisp");
        if (ImGui::MenuItem("JavaScript", nullptr, state.active() && state.active()->language == "javascript"))
            state.setLanguage("javascript");
        if (ImGui::MenuItem("TypeScript", nullptr, state.active() && state.active()->language == "typescript"))
            state.setLanguage("typescript");
        if (ImGui::MenuItem("Java", nullptr, state.active() && state.active()->language == "java"))
            state.setLanguage("java");
        if (ImGui::MenuItem("Rust", nullptr, state.active() && state.active()->language == "rust"))
            state.setLanguage("rust");
        if (ImGui::MenuItem("Go", nullptr, state.active() && state.active()->language == "go"))
            state.setLanguage("go");
        if (ImGui::MenuItem("Org", nullptr, state.active() && state.active()->language == "org"))
            state.setLanguage("org");
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Refactor")) {
        bool canRefactor = state.isStructured() && state.active() && !state.active()->readOnly;
        if (!canRefactor) ImGui::BeginDisabled();
        if (ImGui::MenuItem("Rename Variable")) {
            state.refactorAction = 1;
            state.showRefactorPopup = true;
            state.refactorNameA[0] = '\0';
            state.refactorNameB[0] = '\0';
            ImGui::OpenPopup("RefactorPopup");
        }
        if (ImGui::MenuItem("Extract Function")) {
            state.refactorAction = 2;
            state.showRefactorPopup = true;
            state.refactorNameA[0] = '\0';
            state.refactorNameB[0] = '\0';
            ImGui::OpenPopup("RefactorPopup");
        }
        if (ImGui::MenuItem("Inline Variable")) {
            state.refactorAction = 3;
            state.showRefactorPopup = true;
            state.refactorNameA[0] = '\0';
            state.refactorNameB[0] = '\0';
            ImGui::OpenPopup("RefactorPopup");
        }
        if (!canRefactor) ImGui::EndDisabled();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Workflow")) {
        bool hasAst = state.activeAST() != nullptr;
        if (!hasAst) ImGui::BeginDisabled();
        if (ImGui::MenuItem("Annotate File Wizard")) {
            state.ui.showAnnotateWizard = true;
        }
        if (ImGui::MenuItem("Cross-Language Project Wizard")) {
            state.ui.showProjectWizard = true;
        }
        if (!hasAst) ImGui::EndDisabled();
        if (ImGui::MenuItem("Connect Agent Wizard")) {
            state.ui.showAgentWizard = true;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Keybindings")) {
        for (auto p : KeybindingManager::availableProfiles()) {
            if (ImGui::MenuItem(KeybindingManager::profileName(p), nullptr,
                                state.keys.getProfile() == p)) {
                state.keys.setProfile(p);
                state.registerCommands();
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
        ImGui::MenuItem("Documentation", nullptr, &state.ui.showHelpPanel);
        ImGui::EndMenu();
    }

    const bool canProject = state.activeAST() != nullptr;
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12.0f, 0.0f));
    ImGui::SameLine();
    if (!canProject) ImGui::BeginDisabled();
    if (ImGui::Button("Project to...")) {
        ImGui::OpenPopup("ProjectToPopup");
    }
    if (ImGui::BeginPopup("ProjectToPopup")) {
        if (ImGui::MenuItem("Python")) state.projectToLanguage("python");
        if (ImGui::MenuItem("C++")) state.projectToLanguage("cpp");
        if (ImGui::MenuItem("Elisp")) state.projectToLanguage("elisp");
        ImGui::EndPopup();
    }
    if (!canProject) ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12.0f, 0.0f));
    ImGui::SameLine();
    bool canRunFile = state.active() && !state.active()->readOnly;
    if (!canRunFile) ImGui::BeginDisabled();
    if (ImGui::Button("Run >")) {
        state.runActiveFile(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Build")) {
        state.runActiveFile(true);
    }
    if (!canRunFile) ImGui::EndDisabled();
    ImGui::EndMainMenuBar();
}
