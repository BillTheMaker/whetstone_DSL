#pragma once
// --- BufferOps.h ---
// Extracted from EditorState.h (Sprint 8, Step 236).
// Buffer, file, project, and session operations.
// Included from EditorState.h after the EditorState struct definition.

inline std::string EditorState::makeUntitledName() const {
    if (!buffers.hasBuffer("(untitled)")) return "(untitled)";
    int i = 1;
    while (true) {
        std::string name = "(untitled-" + std::to_string(i) + ")";
        if (!buffers.hasBuffer(name)) return name;
        ++i;
    }
}

inline BufferManager::BufferMode EditorState::defaultBufferMode() const {
    return bufferModeFromString(settings.getDefaultBufferMode());
}

inline void EditorState::createBuffer(const std::string& path, const std::string& content,
    const std::string& language,
    BufferManager::BufferMode mode,
    size_t fileSizeBytes,
    bool largeFileMode,
    bool disableSyntaxHighlight,
    bool deferAstSync) {
    BufferManager::BufferMode effectiveMode = mode;
    if (language == "org") effectiveMode = BufferManager::BufferMode::Text;
    if (buffers.hasBuffer(path)) {
        buffers.switchToBuffer(path);
        activeBuffer = bufferStates[path].get();
        return;
    }
    auto state = std::make_unique<BufferState>();
    state->path = path;
    state->language = language;
    state->generatedLanguage = language;
    state->fileSizeBytes = fileSizeBytes;
    state->largeFileMode = largeFileMode;
    state->disableSyntaxHighlight = disableSyntaxHighlight;
    state->mode.setLanguage(language);
    state->generatedMode.setLanguage(language);
    state->editor.setContent(content, language);
    if (effectiveMode == BufferManager::BufferMode::Structured) {
        if (deferAstSync) {
            state->pendingAstSync = true;
            state->pendingAstStart = ImGui::GetTime();
        } else {
            state->sync.setText(content, language);
            state->sync.syncNow();
            state->incrementalOptimizer.setRoot(state->sync.getAST());
        }
    }
    state->editBuf = content;
    state->highlightsDirty = true;
    state->generatedHighlightsDirty = true;
    state->modified = false;
    state->lspVersion = 1;
    buffers.openBuffer(path, content, language, effectiveMode);
    state->bufferMode = effectiveMode;
    activeBuffer = state.get();
    bufferStates[path] = std::move(state);
    active()->orchestratorDirty = true;
    recordUndoSnapshot();
    if (language == "elisp") {
        emacsState.emacsFunctionIndexDirty = true;
    }
    if (path.rfind("(untitled", 0) != 0) watcher.watch(path);
    if (lsp && path.rfind("(untitled", 0) != 0) {
        lsp->didOpen(toFileUri(path), language, content, activeBuffer->lspVersion);
    }
    events.publish(UIEventType::BufferSwitched, path, {}, ImGui::GetTime());
    uiAnimations.recordTabSwitch(path, ImGui::GetTime());
}

inline void EditorState::switchToBuffer(const std::string& path) {
    if (!buffers.hasBuffer(path)) return;
    buffers.switchToBuffer(path);
    activeBuffer = bufferStates[path].get();
    if (active()) active()->orchestratorDirty = true;
    events.publish(UIEventType::BufferSwitched, path, {}, ImGui::GetTime());
    uiAnimations.recordTabSwitch(path, ImGui::GetTime());
}

inline std::filesystem::path EditorState::configDir() const {
    const char* home = std::getenv("USERPROFILE");
    if (!home) home = std::getenv("HOME");
    std::filesystem::path base = home ? std::filesystem::path(home) : std::filesystem::current_path();
    return base / ".whetstone";
}

inline std::filesystem::path EditorState::recentFilePath() const {
    return configDir() / "recent.json";
}

inline void EditorState::loadRecentFiles() {
    std::filesystem::path p = recentFilePath();
    if (!std::filesystem::exists(p)) return;
    std::ifstream in(p.string());
    if (!in.is_open()) return;
    nlohmann::json j;
    in >> j;
    if (!j.is_array()) return;
    for (const auto& item : j) {
        if (!item.contains("path")) continue;
        std::string path = item.value("path", "");
        std::string lang = item.value("language", "");
        std::string mode = item.value("mode", "structured");
        if (!path.empty()) welcome.addRecentFile(path, lang, mode);
    }
}

inline void EditorState::saveRecentFiles() {
    std::filesystem::path p = recentFilePath();
    std::filesystem::create_directories(p.parent_path());
    nlohmann::json j = nlohmann::json::array();
    for (const auto& rf : welcome.getRecentFiles()) {
        j.push_back({{"path", rf.path}, {"language", rf.language}, {"mode", rf.mode}});
    }
    std::ofstream out(p.string(), std::ios::binary);
    out << j.dump(2);
}

inline void EditorState::closeAllBuffers() {
    auto open = buffers.getOpenBuffers();
    for (const auto& path : open) {
        buffers.closeBuffer(path);
        bufferStates.erase(path);
        if (path.rfind("(untitled", 0) != 0) watcher.unwatch(path);
    }
    activeBuffer = nullptr;
}

inline bool EditorState::renameBufferPath(const std::string& oldPath, const std::string& newPath) {
    if (oldPath == newPath) return false;
    if (!buffers.hasBuffer(oldPath) || buffers.hasBuffer(newPath)) return false;
    auto it = bufferStates.find(oldPath);
    if (it == bufferStates.end()) return false;
    auto node = std::move(it->second);
    bufferStates.erase(it);
    node->path = newPath;
    bufferStates[newPath] = std::move(node);
    buffers.renameBuffer(oldPath, newPath);
    if (oldPath.rfind("(untitled", 0) != 0) watcher.unwatch(oldPath);
    if (newPath.rfind("(untitled", 0) != 0) watcher.watch(newPath);
    if (active() && active()->path == oldPath) {
        active()->path = newPath;
    }
    notify(NotificationLevel::Success, "Renamed buffer to " + newPath);
    return true;
}

inline bool EditorState::saveProject(const std::string& path) {
    ProjectFile project;
    project.workspaceRoot = workspaceRoot;
    project.activePath = active() ? active()->path : "";
    for (const auto& bufPath : buffers.getOpenBuffers()) {
        auto it = bufferStates.find(bufPath);
        if (it == bufferStates.end()) continue;
        ProjectBufferInfo info;
        info.path = bufPath;
        info.language = it->second->language;
        info.mode = bufferModeToString(it->second->bufferMode);
        project.buffers.push_back(std::move(info));
    }
    if (isStructured() && active()) {
        syncOrchestratorFromActive();
        if (active()->orchestrator.getAST()) {
            project.ast = cloneModule(active()->orchestrator.getAST());
        }
    }

    try {
        nlohmann::json j = projectToJson(project);
        std::ofstream out(path, std::ios::binary);
        if (!out.is_open()) return false;
        out << j.dump(2);
        return true;
    } catch (...) {
        return false;
    }
}

inline bool EditorState::loadProject(const std::string& path) {
    try {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) return false;
        nlohmann::json j;
        in >> j;
        ProjectFile project = projectFromJson(j);
        closeAllBuffers();

        if (!project.workspaceRoot.empty()) {
            workspaceRoot = project.workspaceRoot;
            fileTreeDirty = true;
            search.projectSearch.setRoot(workspaceRoot);
        }

        std::string activePath = project.activePath;
        for (const auto& buf : project.buffers) {
            if (buf.path == activePath && project.ast) {
                std::string lang = buf.language.empty() ? project.ast->targetLanguage : buf.language;
                std::string generated = generateForLanguage(project.ast.get(), lang);
                createBuffer(buf.path, generated, lang, bufferModeFromString(buf.mode));
                if (active()) {
                    active()->sync.setAST(std::move(project.ast));
                    active()->incrementalOptimizer.setRoot(active()->sync.getAST());
                    active()->editBuf = active()->sync.getText();
                    active()->editor.setContent(active()->editBuf, lang);
                    active()->mode.setLanguage(lang);
                    active()->highlightsDirty = true;
                    active()->generatedHighlightsDirty = true;
                    active()->modified = false;
                    recordUndoSnapshot();
                }
            } else {
                doOpen(buf.path, bufferModeFromString(buf.mode));
            }
        }

        if (!activePath.empty() && buffers.hasBuffer(activePath)) {
            switchToBuffer(activePath);
        }
        if (active()) active()->orchestratorDirty = true;
        return true;
    } catch (...) {
        return false;
    }
}

inline std::filesystem::path EditorState::sessionFilePath() const {
    return configDir() / "session.json";
}

inline std::filesystem::path EditorState::settingsFilePath() const {
    return configDir() / "settings.json";
}

inline bool EditorState::saveSession(const std::string& imguiIni) {
    SessionData session;
    session.workspaceRoot = workspaceRoot;
    session.activePath = active() ? active()->path : "";
    session.layoutPreset = LayoutManager::presetName(ui.layoutPreset);
    session.imguiIni = imguiIni;
    for (const auto& bufPath : buffers.getOpenBuffers()) {
        auto it = bufferStates.find(bufPath);
        if (it == bufferStates.end()) continue;
        const auto* buf = it->second.get();
        SessionBufferState entry;
        entry.path = bufPath;
        entry.language = buf->language;
        entry.mode = bufferModeToString(buf->bufferMode);
        entry.cursorLine = buf->cursorLine;
        entry.cursorCol = buf->cursorCol;
        entry.foldedLines = buf->widget.getFoldedLines();
        session.buffers.push_back(std::move(entry));
    }
    try {
        std::ofstream out(sessionFilePath().string(), std::ios::binary);
        if (!out.is_open()) return false;
        out << sessionToJson(session).dump(2);
        return true;
    } catch (...) {
        return false;
    }
}

inline bool EditorState::loadSession(SessionData& out) {
    try {
        std::ifstream in(sessionFilePath().string(), std::ios::binary);
        if (!in.is_open()) return false;
        nlohmann::json j;
        in >> j;
        out = sessionFromJson(j);
        return true;
    } catch (...) {
        return false;
    }
}

inline void EditorState::applySession(const SessionData& session) {
    closeAllBuffers();
    if (!session.workspaceRoot.empty()) {
        workspaceRoot = session.workspaceRoot;
        fileTreeDirty = true;
        search.projectSearch.setRoot(workspaceRoot);
    }
    ui.layoutPreset = LayoutManager::presetFromName(session.layoutPreset);
    for (const auto& buf : session.buffers) {
        doOpen(buf.path, bufferModeFromString(buf.mode), true);
        auto it = bufferStates.find(buf.path);
        if (it != bufferStates.end()) {
            auto* bs = it->second.get();
            bs->cursorLine = buf.cursorLine;
            bs->cursorCol = buf.cursorCol;
            bs->widget.setDesiredFoldedLines(buf.foldedLines);
            jumpTo(bs, std::max(0, buf.cursorLine - 1), std::max(0, buf.cursorCol - 1));
        }
    }
    if (!session.activePath.empty() && buffers.hasBuffer(session.activePath)) {
        switchToBuffer(session.activePath);
    }
}

inline void EditorState::applySettingsToState() {
    ui.showMinimap = settings.getShowMinimap();
    ui.showCompletionHelper = settings.getShowCompletionHelper();
    ui.showLineNumbers = settings.getShowLineNumbers();
    ui.layoutPreset = LayoutManager::presetFromName(settings.getLayoutPreset());
    keys.setProfile(KeybindingManager::profileFromName(settings.getKeybindingProfile()));
    registerCommands();
    applyTabSizeToBuffers(settings.getTabSize());
    library.primitives.setBlockVulnerableImports(settings.getBlockVulnerableImports());
}

inline void EditorState::loadSettingsFromDisk() {
    settings.loadFromFile(settingsFilePath().string());
    applySettingsToState();
}

inline void EditorState::saveSettingsToDisk() {
    settings.setShowMinimap(ui.showMinimap);
    settings.setShowCompletionHelper(ui.showCompletionHelper);
    settings.setShowLineNumbers(ui.showLineNumbers);
    settings.setLayoutPreset(LayoutManager::presetName(ui.layoutPreset));
    settings.setKeybindingProfile(KeybindingManager::profileName(keys.getProfile()));
    std::filesystem::create_directories(settingsFilePath().parent_path());
    settings.saveToFile(settingsFilePath().string());
}

inline void EditorState::doSave() {
    if (!active()) return;
    if (active()->path.rfind("(untitled", 0) == 0) return;
    std::ofstream out(active()->path);
    if (out.is_open()) {
        out << active()->editBuf;
        out.close();
        active()->modified = false;
        notify(NotificationLevel::Success,
               "Saved: " + active()->path);
        recordEditorEvent("file_saved", {{"path", active()->path}});
        if (lsp) lsp->didSave(toFileUri(active()->path), active()->editBuf);
    } else {
        notify(NotificationLevel::Error,
               "Error saving: " + active()->path);
    }
}

inline void EditorState::doOpen(const std::string& path,
    BufferManager::BufferMode modeOverride,
    bool deferAstSync) {
    std::error_code sizeErr;
    size_t fileSizeBytes = 0;
    if (!path.empty()) {
        auto bytes = std::filesystem::file_size(path, sizeErr);
        if (!sizeErr) fileSizeBytes = static_cast<size_t>(bytes);
    }
    const size_t mb = 1024 * 1024;
    const size_t warnBytes = (size_t)std::max(1, settings.getLargeFileWarnMB()) * mb;
    const size_t textBytes = (size_t)std::max(1, settings.getLargeFileTextMB()) * mb;
    const size_t disableHlBytes =
        (size_t)std::max(1, settings.getLargeFileDisableHighlightMB()) * mb;
    bool warnLarge = fileSizeBytes >= warnBytes;
    bool autoText = fileSizeBytes >= textBytes;
    bool disableHighlight = fileSizeBytes >= disableHlBytes;
    bool largeFileMode = disableHighlight;
    BufferManager::BufferMode effectiveMode = modeOverride;
    if (autoText && modeOverride == BufferManager::BufferMode::Structured) {
        effectiveMode = BufferManager::BufferMode::Text;
    }
    std::ifstream in(path);
    if (in.is_open()) {
        std::ostringstream ss;
        ss << in.rdbuf();
        std::string content = ss.str();
        std::string language = "python";
        if (path.size() > 3 && path.substr(path.size() - 3) == ".py")
            language = "python";
        else if (path.size() > 4 && path.substr(path.size() - 4) == ".cpp")
            language = "cpp";
        else if (path.size() > 2 && path.substr(path.size() - 2) == ".h")
            language = "cpp";
        else if (path.size() > 3 && path.substr(path.size() - 3) == ".el")
            language = "elisp";
        else if (path.size() > 3 && path.substr(path.size() - 3) == ".js")
            language = "javascript";
        else if (path.size() > 3 && path.substr(path.size() - 3) == ".ts")
            language = "typescript";
        else if (path.size() > 5 && path.substr(path.size() - 5) == ".java")
            language = "java";
        else if (path.size() > 3 && path.substr(path.size() - 3) == ".rs")
            language = "rust";
        else if (path.size() > 3 && path.substr(path.size() - 3) == ".go")
            language = "go";
        else if (path.size() > 4 && path.substr(path.size() - 4) == ".org")
            language = "org";
        createBuffer(path, content, language, effectiveMode,
                     fileSizeBytes, largeFileMode, disableHighlight, deferAstSync);
        welcome.addRecentFile(path, language, bufferModeToString(effectiveMode));
        saveRecentFiles();
        if (autoText && modeOverride == BufferManager::BufferMode::Structured) {
            notify(NotificationLevel::Warning,
                   "Large file opened in Text mode (" +
                   std::to_string(fileSizeBytes / mb) + " MB).");
        } else if (warnLarge && modeOverride == BufferManager::BufferMode::Structured) {
            showLargeFilePrompt = true;
            largeFilePromptPath = path;
            largeFilePromptBytes = fileSizeBytes;
        }
        notify(NotificationLevel::Success, "Opened: " + path);
        recordEditorEvent("file_opened", {
            {"path", path},
            {"language", language},
            {"mode", bufferModeToString(effectiveMode)},
            {"sizeBytes", fileSizeBytes}
        });
    } else {
        notify(NotificationLevel::Error, "Error opening: " + path);
    }
}

inline void EditorState::reloadBuffer(const std::string& path) {
    auto it = bufferStates.find(path);
    if (it == bufferStates.end()) return;
    std::ifstream in(path);
    if (!in.is_open()) return;
    std::ostringstream ss;
    ss << in.rdbuf();
    auto* buf = it->second.get();
    buf->editBuf = ss.str();
    buf->editor.setContent(buf->editBuf, buf->language);
    if (buf->bufferMode == BufferManager::BufferMode::Structured) {
        buf->sync.setText(buf->editBuf, buf->language);
        buf->sync.syncNow();
        buf->incrementalOptimizer.setRoot(buf->sync.getAST());
    }
    buf->highlightsDirty = true;
    buf->highlightRequestTime = ImGui::GetTime();
    buf->generatedHighlightsDirty = true;
    buf->modified = false;
    notify(NotificationLevel::Info, "Reloaded: " + path);
}

inline void EditorState::handleFileChanges() {
    auto changed = watcher.poll();
    for (const auto& path : changed) {
        auto it = bufferStates.find(path);
        if (it == bufferStates.end()) continue;
        if (it->second->modified) {
            notify(NotificationLevel::Warning,
                   "File changed on disk (dirty): " + path);
        } else {
            reloadBuffer(path);
        }
    }
}

inline void EditorState::updateHighlights() {
    if (!active()) return;
    if (!active()->highlightsDirty) return;
    double now = ImGui::GetTime();
    double delay = settings.getHighlightDebounceMs() / 1000.0;
    if (active()->highlightRequestTime > 0.0 &&
        (now - active()->highlightRequestTime) < delay) {
        return;
    }
    if (active()->disableSyntaxHighlight) {
        active()->highlights.clear();
        active()->highlightsDirty = false;
        return;
    }
    active()->highlights = SyntaxHighlighter::highlight(active()->editBuf, active()->language);
    active()->highlightsDirty = false;
}

inline void EditorState::updateGenerated() {
    if (!active()) return;
    if (active()->bufferMode == BufferManager::BufferMode::Text) return;
    Module* ast = active()->sync.getAST();
    std::string generated = generateForLanguage(ast, active()->generatedLanguage);
    if (generated != active()->generatedBuf) {
        active()->generatedBuf = generated;
        active()->generatedHighlightsDirty = true;
    }
    if (active()->disableSyntaxHighlight) {
        active()->generatedHighlights.clear();
        active()->generatedHighlightsDirty = false;
        return;
    }
    if (active()->generatedHighlightsDirty) {
        active()->generatedHighlights =
            SyntaxHighlighter::highlight(active()->generatedBuf, active()->generatedLanguage);
        active()->generatedHighlightsDirty = false;
    }
}

inline void EditorState::updateCursorPos(int bytePos) {
    if (!active()) return;
    active()->cursorLine = 1;
    active()->cursorCol = 1;
    for (int i = 0; i < bytePos && i < (int)active()->editBuf.size(); ++i) {
        if (active()->editBuf[i] == '\n') { ++active()->cursorLine; active()->cursorCol = 1; }
        else { ++active()->cursorCol; }
    }
}

inline void EditorState::refreshFileTree() {
    if (!fileTreeDirty) return;
    fileTreeRoot = fileTree.build(workspaceRoot);
    fileTreeDirty = false;
}

inline void EditorState::navigateToTarget(const NotificationTarget& target) {
    if (target.path.empty()) return;
    std::string path = target.path;
    if (buffers.hasBuffer(path)) switchToBuffer(path);
    else doOpen(path, defaultBufferMode());
    if (active()) {
        int line = std::max(0, target.line);
        int col = std::max(0, target.col);
        jumpTo(active(), line, col);
    }
}

inline void EditorState::applyCompletion(BufferState* buf, const std::string& insertText, int replaceStart, int replaceEnd) {
    if (!buf) return;
    replaceStart = std::max(0, std::min(replaceStart, (int)buf->editBuf.size()));
    replaceEnd = std::max(replaceStart, std::min(replaceEnd, (int)buf->editBuf.size()));
    buf->editBuf.replace(replaceStart, replaceEnd - replaceStart, insertText);
    int newPos = replaceStart + (int)insertText.size();
    buf->widget.setCursor(newPos);
    activeBuffer = buf;
    onTextChanged();
}

inline void EditorState::insertTextAtCursor(const std::string& text) {
    if (!active() || text.empty()) return;
    int cursor = active()->widget.getCursor();
    cursor = std::max(0, std::min(cursor, (int)active()->editBuf.size()));
    active()->editBuf.insert(cursor, text);
    active()->widget.setCursor(cursor + (int)text.size());
    onTextChanged();
}

// --- Projection and lifecycle helpers (extracted) ---
#include "BufferOpsProjection.h"
#include "BufferOpsLifecycle.h"
