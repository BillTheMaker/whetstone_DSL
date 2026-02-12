#pragma once
// --- EditOps.h ---
// Extracted from EditorState.h (Sprint 8, Step 237).
// Editing operations, undo/redo, find/replace, commands, navigation.
// Included from EditorState.h after the EditorState struct definition.

inline bool EditorState::jumpToDefinitionLocation(const LSPClient::DefinitionLocation& loc) {
    if (loc.uri.empty()) return false;
    std::string path = fromFileUri(loc.uri);
    if (path.empty()) return false;
    if (buffers.hasBuffer(path)) switchToBuffer(path);
    else doOpen(path, defaultBufferMode());
    jumpTo(active(), loc.range.start.line, loc.range.start.character);
    return true;
}

inline bool EditorState::goToDefinitionFallback(int lineZero, int colZero) {
    if (!isStructured()) return false;
    Module* ast = activeAST();
    if (!ast || !active()) return false;
    std::string word = wordAtLineCol(active()->editBuf, lineZero, colZero);
    if (word.empty()) return false;

    ASTNode* scopeNode = findNodeAtPosition(ast, lineZero, colZero);
    if (!scopeNode) scopeNode = ast;

    ContextAPI ctx;
    ctx.setRoot(ast);
    auto symbols = ctx.getInScopeSymbols(scopeNode->id);
    for (const auto& sym : symbols) {
        if (sym.name != word) continue;
        ASTNode* target = findNodeById(ast, sym.nodeId);
        if (target && target->hasSpan()) {
            jumpTo(active(), target->spanStartLine, target->spanStartCol);
            return true;
        }
    }
    return false;
}

inline bool EditorState::goToDefinitionAt(int lineZero, int colZero) {
    if (!active()) return false;
    if (lsp && lspTransport && lspTransport->isOpen() &&
        active()->path.rfind("(untitled", 0) != 0) {
        definitionPreviewValid = false;
        definitionPending = true;
        definitionLastRequest = ImGui::GetTime();
        definitionRequestLine = lineZero;
        definitionRequestCol = colZero;
        definitionRequestPath = active()->path;
        lsp->clearDefinitionLocations();
        lsp->requestDefinition(toFileUri(active()->path), lineZero, colZero);
        return true;
    }
    return goToDefinitionFallback(lineZero, colZero);
}

inline bool EditorState::previewDefinitionAt(int lineZero, int colZero, std::string& outLabel) {
    if (!isStructured() || !active()) return false;
    Module* ast = activeAST();
    if (!ast) return false;
    std::string word = wordAtLineCol(active()->editBuf, lineZero, colZero);
    if (word.empty()) return false;
    ASTNode* scopeNode = findNodeAtPosition(ast, lineZero, colZero);
    if (!scopeNode) scopeNode = ast;
    ContextAPI ctx;
    ctx.setRoot(ast);
    auto symbols = ctx.getInScopeSymbols(scopeNode->id);
    for (const auto& sym : symbols) {
        if (sym.name != word) continue;
        ASTNode* target = findNodeById(ast, sym.nodeId);
        if (target && target->hasSpan()) {
            outLabel = word + " -> line " + std::to_string(target->spanStartLine + 1);
            return true;
        }
    }
    return false;
}

inline void EditorState::applyTabSizeToBuffers(int size) {
    for (auto& kv : bufferStates) {
        kv.second->mode.setTabSize(size);
        kv.second->generatedMode.setTabSize(size);
    }
}

inline void EditorState::setActiveBufferMode(BufferManager::BufferMode mode) {
    if (!active()) return;
    if (active()->bufferMode == mode) return;
    active()->bufferMode = mode;
    buffers.setBufferMode(active()->path, active()->bufferMode);
    if (active()->bufferMode == BufferManager::BufferMode::Text) {
        suggestions.clear();
        whetstoneDiagnostics.clear();
        analysisPending = false;
    } else {
        onTextChanged();
    }
}

inline void EditorState::queueLspDidChange() {
    if (!lsp || !active()) return;
    if (active()->path.rfind("(untitled", 0) == 0) return;
    active()->lspVersion += 1;
    lspChangePending = true;
    lspChangeLast = ImGui::GetTime();
    lspChangePath = active()->path;
}

inline void EditorState::syncOrchestratorFromActive() {
    if (!active()) return;
    if (!isStructured()) return;
    Module* ast = active()->sync.getAST();
    if (!ast) return;
    active()->orchestrator.setAST(cloneModule(ast));
    active()->orchestratorDirty = false;
}

inline void EditorState::applyOrchestratorToActive() {
    if (!active()) return;
    if (!isStructured()) return;
    Module* ast = active()->orchestrator.getAST();
    if (!ast) return;
    active()->sync.setAST(cloneModule(ast));
    active()->incrementalOptimizer.setRoot(active()->sync.getAST());
    refreshActiveTextFromAST();
    active()->orchestratorDirty = false;
    recordUndoSnapshot();
}

inline Module* EditorState::mutationAST() {
    if (!active()) return nullptr;
    if (!isStructured()) return nullptr;
    if (active()->orchestratorDirty || !active()->orchestrator.getAST()) {
        syncOrchestratorFromActive();
    }
    return active()->orchestrator.getAST();
}

inline void EditorState::recordUndoSnapshot() {
    if (!active()) return;
    Module* ast = isStructured() ? active()->sync.getAST() : nullptr;
    active()->orchestrator.recordSnapshot(active()->editBuf, ast);
    active()->undoDepth = active()->orchestrator.getUndoDepth();
}

inline void EditorState::applySnapshotToActive(const std::string& text, std::unique_ptr<Module> ast) {
    if (!active()) return;
    active()->editBuf = text;
    active()->editor.setContent(active()->editBuf, active()->language);
    if (active()->bufferMode == BufferManager::BufferMode::Structured) {
        if (ast) {
            active()->orchestrator.setAST(cloneModule(ast.get()));
            active()->sync.setAST(std::move(ast));
        } else {
            active()->sync.setText(active()->editBuf, active()->language);
            active()->sync.syncNow();
            active()->orchestrator.setAST(cloneModule(active()->sync.getAST()));
        }
        active()->incrementalOptimizer.setRoot(active()->sync.getAST());
    }
    active()->orchestratorDirty = false;
    active()->highlightsDirty = true;
    active()->highlightRequestTime = ImGui::GetTime();
    active()->generatedHighlightsDirty = true;
    active()->modified = true;
    queueLspDidChange();
}

// --- Command registration (extracted) ---
#include "EditOpsCommands.h"

inline void EditorState::setLanguage(const std::string& lang) {
    if (!active()) return;
    std::string prevLang = active()->language;
    active()->language = lang;
    active()->mode.setLanguage(lang);
    if (activeAST()) {
        library.primitives.setRoot(activeAST());
    }
    library.primitives.setLanguage(lang);
    if (active()->generatedLanguage == prevLang) {
        active()->generatedLanguage = lang;
        active()->generatedMode.setLanguage(lang);
        active()->generatedHighlightsDirty = true;
    }
    active()->editor.setContent(active()->editBuf, lang);
    if (active()->bufferMode == BufferManager::BufferMode::Structured) {
        active()->sync.setText(active()->editBuf, lang);
        active()->sync.syncNow();
        active()->incrementalOptimizer.setRoot(active()->sync.getAST());
    }
    active()->orchestratorDirty = true;
    active()->highlightsDirty = true;
    if (lang == "elisp") {
        emacsState.emacsFunctionIndexDirty = true;
    }
    if (lang == "org") {
        active()->bufferMode = BufferManager::BufferMode::Text;
        buffers.setBufferMode(active()->path, active()->bufferMode);
    }
    events.publishDebounced(UIEventType::ASTChanged,
                            active()->path,
                            {},
                            ImGui::GetTime(),
                            0.1);
}

inline void EditorState::onTextChanged() {
    if (!active()) return;
    active()->editor.setContent(active()->editBuf, active()->language);
    if (active()->bufferMode == BufferManager::BufferMode::Structured) {
        active()->sync.setText(active()->editBuf, active()->language);
        active()->sync.syncNow();
        active()->incrementalOptimizer.setRoot(active()->sync.getAST());
    }
    active()->orchestratorDirty = true;
    active()->highlightsDirty = true;
    active()->highlightRequestTime = ImGui::GetTime();
    active()->generatedHighlightsDirty = true;
    active()->modified = true;
    queueLspDidChange();
    events.publish(UIEventType::FileModified, active()->path, {}, ImGui::GetTime());
    events.publishDebounced(UIEventType::ASTChanged,
                            active()->path,
                            {},
                            ImGui::GetTime(),
                            0.15);
    recordUndoSnapshot();
}

inline void EditorState::doUndo() {
    if (!active()) return;
    std::string text;
    std::unique_ptr<Module> ast;
    if (active()->orchestrator.undoSnapshot(text, ast)) {
        applySnapshotToActive(text, std::move(ast));
    }
    active()->undoDepth = active()->orchestrator.getUndoDepth();
}

inline void EditorState::doRedo() {
    if (!active()) return;
    std::string text;
    std::unique_ptr<Module> ast;
    if (active()->orchestrator.redoSnapshot(text, ast)) {
        applySnapshotToActive(text, std::move(ast));
    }
    active()->undoDepth = active()->orchestrator.getUndoDepth();
}

inline SearchOptions EditorState::currentSearchOptions() const {
    SearchOptions opts;
    opts.matchCase = search.matchCase;
    opts.wholeWord = search.wholeWord;
    opts.useRegex = search.useRegex;
    return opts;
}

inline void EditorState::addSearchHistory(const std::string& query) {
    if (query.empty()) return;
    auto it = std::find(search.findHistory.begin(), search.findHistory.end(), query);
    if (it != search.findHistory.end()) {
        search.findHistory.erase(it);
    }
    search.findHistory.insert(search.findHistory.begin(), query);
    if (search.findHistory.size() > 10) {
        search.findHistory.resize(10);
    }
}

inline bool EditorState::selectionRange(int& outStart, int& outEnd) {
    if (!active()) return false;
    if (!active()->widget.hasSelectionRange()) return false;
    active()->widget.getSelectionRange(outStart, outEnd);
    return outStart >= 0 && outEnd > outStart;
}

inline void EditorState::refreshSearchMatches() {
    search.matches.clear();
    search.currentMatchIndex = -1;
    if (!active()) return;
    std::string query = search.findBuf;
    if (query.empty()) return;
    int rangeStart = 0;
    int rangeEnd = (int)active()->editBuf.size();
    if (search.findInSelection) {
        int selStart = -1;
        int selEnd = -1;
        if (selectionRange(selStart, selEnd)) {
            rangeStart = selStart;
            rangeEnd = selEnd;
        }
    }
    search.matches = SearchUtils::collectMatches(active()->editBuf,
                                                 query,
                                                 currentSearchOptions(),
                                                 rangeStart,
                                                 rangeEnd);
}

inline bool EditorState::moveToMatchIndex(int index) {
    if (!active()) return false;
    if (index < 0 || index >= (int)search.matches.size()) return false;
    const auto& match = search.matches[index];
    active()->widget.setSelectionRange(match.start, match.end);
    active()->widget.setCursor(match.end);
    updateCursorPos(match.start);
    search.currentMatchIndex = index;
    search.lastFindPos = match.end;
    search.pulseLine = match.line;
    search.pulseStart = ImGui::GetTime();
    return true;
}

inline void EditorState::doFindNext(bool backwards) {
    if (!active()) return;
    if (strlen(search.findBuf) == 0) return;
    refreshSearchMatches();
    if (search.matches.empty()) {
        search.lastFindPos = 0;
        search.pulseLine = -1;
        search.pulseStart = 0.0;
        notify(NotificationLevel::Warning,
               "\"" + std::string(search.findBuf) + "\" not found.");
        return;
    }

    int cursorPos = active()->widget.getCursor();
    int chosenIndex = -1;
    if (backwards) {
        for (int i = (int)search.matches.size() - 1; i >= 0; --i) {
            if (search.matches[i].end < cursorPos) {
                chosenIndex = i;
                break;
            }
        }
        if (chosenIndex < 0) chosenIndex = (int)search.matches.size() - 1;
    } else {
        for (int i = 0; i < (int)search.matches.size(); ++i) {
            if (search.matches[i].start > cursorPos) {
                chosenIndex = i;
                break;
            }
        }
        if (chosenIndex < 0) chosenIndex = 0;
    }

    if (moveToMatchIndex(chosenIndex)) {
        addSearchHistory(search.findBuf);
        const auto& m = search.matches[chosenIndex];
        NotificationTarget target;
        target.path = active()->path;
        target.line = m.line;
        target.col = m.col;
        notify(NotificationLevel::Info,
               "Found \"" + std::string(search.findBuf) + "\" at line " +
               std::to_string(m.line + 1) + ", col " + std::to_string(m.col + 1),
               target);
    }
}

inline void EditorState::doReplaceCurrent() {
    if (!active()) return;
    if (strlen(search.findBuf) == 0) return;
    refreshSearchMatches();
    if (search.matches.empty()) {
        notify(NotificationLevel::Warning,
               "\"" + std::string(search.findBuf) + "\" not found.");
        return;
    }

    int index = search.currentMatchIndex;
    if (index < 0 || index >= (int)search.matches.size()) {
        int cursorPos = active()->widget.getCursor();
        for (int i = 0; i < (int)search.matches.size(); ++i) {
            if (search.matches[i].start >= cursorPos) {
                index = i;
                break;
            }
        }
        if (index < 0) index = 0;
    }

    const auto& match = search.matches[index];
    std::string replacement = SearchUtils::applyReplacement(match.matchText,
                                                            search.findBuf,
                                                            search.replaceBuf,
                                                            currentSearchOptions());
    active()->editBuf.replace(match.start, match.end - match.start, replacement);
    onTextChanged();
    active()->widget.setCursor(match.start + (int)replacement.size());
    updateCursorPos(match.start + (int)replacement.size());
    addSearchHistory(search.findBuf);
}

inline void EditorState::doFind() {
    doFindNext(false);
}

inline void EditorState::doReplaceAll() {
    if (!active()) return;
    if (strlen(search.findBuf) == 0) return;
    int rangeStart = 0;
    int rangeEnd = (int)active()->editBuf.size();
    if (search.findInSelection) {
        int selStart = -1;
        int selEnd = -1;
        if (selectionRange(selStart, selEnd)) {
            rangeStart = selStart;
            rangeEnd = selEnd;
        }
    }
    int count = 0;
    std::string replaced = SearchUtils::replaceAll(active()->editBuf,
                                                   search.findBuf,
                                                   search.replaceBuf,
                                                   currentSearchOptions(),
                                                   rangeStart,
                                                   rangeEnd,
                                                   count);
    if (count > 0) {
        active()->editBuf = replaced;
        onTextChanged();
    }
    addSearchHistory(search.findBuf);
    notify(NotificationLevel::Info,
           "Replaced " + std::to_string(count) + " occurrence(s).");
}

inline std::string EditorState::fromFileUri(const std::string& uri) {
    const std::string prefix = "file:///";
    if (uri.rfind(prefix, 0) != 0) return uri;
    std::string path = uri.substr(prefix.size());
    std::replace(path.begin(), path.end(), '/', '\\');
    return path;
}

inline int EditorState::byteOffsetForLineCol(const std::string& text, int lineZero, int colZero) {
    if (lineZero < 0) lineZero = 0;
    if (colZero < 0) colZero = 0;
    int line = 0;
    int pos = 0;
    while (pos < (int)text.size() && line < lineZero) {
        if (text[pos] == '\n') ++line;
        ++pos;
    }
    int col = 0;
    while (pos < (int)text.size() && text[pos] != '\n' && col < colZero) {
        ++pos;
        ++col;
    }
    return pos;
}

inline int EditorState::wordStartAt(const std::string& text, int pos) {
    pos = std::max(0, std::min(pos, (int)text.size()));
    while (pos > 0) {
        char c = text[pos - 1];
        if (std::isalnum((unsigned char)c) || c == '_') {
            --pos;
        } else {
            break;
        }
    }
    return pos;
}

inline std::string EditorState::wordAt(const std::string& text, int pos) {
    if (text.empty()) return "";
    pos = std::max(0, std::min(pos, (int)text.size()));
    if (pos == (int)text.size()) pos = (int)text.size() - 1;
    if (pos < 0) return "";
    if (!isWordChar(text[pos]) && pos > 0 && isWordChar(text[pos - 1])) {
        --pos;
    }
    if (!isWordChar(text[pos])) return "";
    int start = pos;
    while (start > 0 && isWordChar(text[start - 1])) --start;
    int end = pos;
    while (end < (int)text.size() && isWordChar(text[end])) ++end;
    return text.substr(start, end - start);
}

inline std::string EditorState::wordAtLineCol(const std::string& text, int lineZero, int colZero) {
    int pos = byteOffsetForLineCol(text, lineZero, colZero);
    return wordAt(text, pos);
}

inline std::string EditorState::wordPrefixAt(const std::string& text, int pos) {
    int start = wordStartAt(text, pos);
    return text.substr(start, pos - start);
}

inline std::string EditorState::uiEventLabel(UIEventType type) {
    switch (type) {
        case UIEventType::ASTChanged: return "ast_changed";
        case UIEventType::BufferSwitched: return "buffer_switched";
        case UIEventType::DiagnosticsUpdated: return "diagnostics_updated";
        case UIEventType::ThemeChanged: return "theme_changed";
        case UIEventType::SettingsChanged: return "settings_changed";
        case UIEventType::FileModified: return "file_modified";
        case UIEventType::AgentConnected: return "agent_connected";
        case UIEventType::NotificationPosted: return "notification_posted";
    }
    return "unknown";
}
