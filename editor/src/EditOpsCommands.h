#pragma once
// --- EditOpsCommands.h ---
// Extracted from EditOps.h (Sprint 8 cleanup).
// Command registration and execution.
// Included from EditOps.h after the EditorState struct definition.

inline void EditorState::registerCommand(const std::string& id,
    const std::string& label,
    const std::string& shortcut,
    const std::string& category,
    int contextMask,
    std::function<void()> fn) {
    commandPalette.registerCommand(id, label, shortcut, category, contextMask, "", false);
    commandHandlers[id] = [fn](const std::string&) { fn(); };
}

inline void EditorState::registerCommand(const std::string& id,
    const std::string& label,
    const std::string& shortcut,
    const std::string& category,
    int contextMask,
    const std::string& inputHint,
    std::function<void(const std::string&)> fn) {
    commandPalette.registerCommand(id, label, shortcut, category, contextMask, inputHint, true);
    commandHandlers[id] = std::move(fn);
}

inline void EditorState::registerCommands() {
    commandPalette = CommandPalette();
    commandHandlers.clear();

    registerCommand("file.new", "File: New File",
                    keys.getBinding("file.new").toString(),
                    "File", CommandContext_Editor,
                    [this]() {
                        std::string lang = active() ? active()->language : "python";
                        createBuffer(makeUntitledName(), "", lang, defaultBufferMode());
                    });
    registerCommand("file.open", "File: Open...",
                    keys.getBinding("file.open").toString(),
                    "File", CommandContext_Editor,
                    [this]() {
                        std::string path = FileDialog::openFile(
                            {"Open File", {"*.py","*.cpp","*.h","*.el","*.js","*.ts","*.java","*.rs","*.go"},
                             std::string()});
                        if (!path.empty()) doOpen(path, defaultBufferMode());
                    });
    registerCommand("file.save", "File: Save",
                    keys.getBinding("file.save").toString(),
                    "File", CommandContext_Editor,
                    [this]() { doSave(); });
    registerCommand("project.open", "Project: Open...",
                    "",
                    "Project", CommandContext_Editor,
                    [this]() {
                        std::string path = FileDialog::openFile(
                            {"Open Project", {"*.whetstone"}, std::string()});
                        if (!path.empty()) {
                            if (!loadProject(path)) {
                                notify(NotificationLevel::Error,
                                       "Failed to open project: " + path);
                            }
                        }
                    });
    registerCommand("project.save", "Project: Save...",
                    "",
                    "Project", CommandContext_Editor,
                    [this]() {
                        std::string path = FileDialog::saveFile(
                            {"Save Project", {"*.whetstone"}, std::string()});
                        if (!path.empty()) {
                            if (!saveProject(path)) {
                                notify(NotificationLevel::Error,
                                       "Failed to save project: " + path);
                            }
                        }
                    });
    registerCommand("edit.undo", "Edit: Undo",
                    keys.getBinding("edit.undo").toString(),
                    "Edit", CommandContext_Editor,
                    [this]() { doUndo(); });
    registerCommand("edit.redo", "Edit: Redo",
                    keys.getBinding("edit.redo").toString(),
                    "Edit", CommandContext_Editor,
                    [this]() { doRedo(); });
    registerCommand("search.find", "Search: Find/Replace",
                    keys.getBinding("search.find").toString(),
                    "Search", CommandContext_Editor,
                    [this]() {
                        search.showFind = true;
                        search.showReplace = false;
                    });
    registerCommand("search.replace", "Search: Replace",
                    keys.getBinding("search.replace").toString(),
                    "Search", CommandContext_Editor,
                    [this]() {
                        search.showFind = true;
                        search.showReplace = true;
                    });
    registerCommand("search.findNext", "Search: Find Next",
                    keys.getBinding("search.findNext").toString(),
                    "Search", CommandContext_Editor,
                    [this]() { doFindNext(false); });
    registerCommand("search.findPrev", "Search: Find Previous",
                    keys.getBinding("search.findPrev").toString(),
                    "Search", CommandContext_Editor,
                    [this]() { doFindNext(true); });
    registerCommand("search.findInFiles", "Search: Find in Files",
                    keys.getBinding("search.findInFiles").toString(),
                    "Search", CommandContext_Search,
                    [this]() { search.showProjectSearch = !search.showProjectSearch; });
    registerCommand("nav.goToLine", "Navigate: Go to Line",
                    keys.getBinding("nav.goToLine").toString(),
                    "Navigate", CommandContext_Editor, "line[:col]",
                    [this](const std::string& arg) {
                        if (!arg.empty()) {
                            int line = 0;
                            int col = 0;
                            if (parseLineColInput(arg, line, col)) {
                                if (active()) {
                                    int totalLines = countLines(active()->editBuf);
                                    if (totalLines > 0) line = std::max(1, std::min(line, totalLines));
                                    col = std::max(1, col);
                                    jumpTo(active(), line - 1, col - 1);
                                }
                            } else {
                                notify(NotificationLevel::Warning, "Go to Line: invalid input.");
                            }
                            return;
                        }
                        search.showGoToLine = true;
                        search.goToLineBuf[0] = '\0';
                        search.goToLineError = false;
                    });
    registerCommand("view.whitespace", "View: Toggle Whitespace", "",
                    "View", CommandContext_Editor,
                    [this]() { ui.showWhitespace = !ui.showWhitespace; });
    registerCommand("view.minimap", "View: Toggle Minimap", "",
                    "View", CommandContext_Editor,
                    [this]() { ui.showMinimap = !ui.showMinimap; });
    registerCommand("view.annotations", "View: Toggle Annotations", "",
                    "View", CommandContext_Editor,
                    [this]() { ui.showAnnotations = !ui.showAnnotations; });
    registerCommand("view.outline", "View: Toggle Outline", "",
                    "View", CommandContext_Editor,
                    [this]() { ui.showOutline = !ui.showOutline; });
    registerCommand("view.toggleTerminal", "View: Toggle Terminal",
                    keys.getBinding("view.toggleTerminal").toString(),
                    "View", CommandContext_Terminal,
                    [this]() { build.showTerminalPanel = !build.showTerminalPanel; });
    registerCommand("view.lsp", "View: LSP Servers...", "",
                    "View", CommandContext_Settings,
                    [this]() { ui.showLspSettings = !ui.showLspSettings; });
    registerCommand("mode.toggle", "Mode: Toggle Text/Structured", "",
                    "Mode", CommandContext_Editor,
                    [this]() {
                        if (!active()) return;
                        active()->bufferMode = active()->bufferMode == BufferManager::BufferMode::Text ?
                            BufferManager::BufferMode::Structured : BufferManager::BufferMode::Text;
                        buffers.setBufferMode(active()->path, active()->bufferMode);
                        if (active()->bufferMode == BufferManager::BufferMode::Text) {
                            suggestions.clear();
                            whetstoneDiagnostics.clear();
                            analysisPending = false;
                        } else {
                            onTextChanged();
                        }
                    });
    registerCommand("refactor.rename", "Refactor: Rename Variable", "",
                    "Refactor", CommandContext_Editor,
                    [this]() {
                        refactorAction = 1;
                        showRefactorPopup = true;
                        refactorNameA[0] = '\0';
                        refactorNameB[0] = '\0';
                    });
    registerCommand("refactor.extract", "Refactor: Extract Function", "",
                    "Refactor", CommandContext_Editor,
                    [this]() {
                        refactorAction = 2;
                        showRefactorPopup = true;
                        refactorNameA[0] = '\0';
                        refactorNameB[0] = '\0';
                    });
    registerCommand("refactor.inline", "Refactor: Inline Variable", "",
                    "Refactor", CommandContext_Editor,
                    [this]() {
                        refactorAction = 3;
                        showRefactorPopup = true;
                        refactorNameA[0] = '\0';
                        refactorNameB[0] = '\0';
                    });
    registerCommand("wizard.annotate", "Wizard: Annotate File", "",
                    "Workflow", CommandContext_Editor,
                    [this]() { ui.showAnnotateWizard = true; });
    registerCommand("wizard.project", "Wizard: Cross-Language Project", "",
                    "Workflow", CommandContext_Editor,
                    [this]() { ui.showProjectWizard = true; });
    registerCommand("wizard.agent", "Wizard: Connect Agent", "",
                    "Workflow", CommandContext_Editor,
                    [this]() { ui.showAgentWizard = true; });
    registerCommand("build.run", "Build: Run", keys.getBinding("build.run").toString(),
                    "Build", CommandContext_Terminal,
                    [this]() { runActiveFile(false); });
    registerCommand("build.build", "Build: Build", keys.getBinding("build.build").toString(),
                    "Build", CommandContext_Terminal,
                    [this]() { runActiveFile(true); });
}

inline void EditorState::executeCommand(const std::string& id, const std::string& arg = {}) {
    auto it = commandHandlers.find(id);
    if (it == commandHandlers.end()) return;
    it->second(arg);
    commandPalette.markUsed(id);
}
