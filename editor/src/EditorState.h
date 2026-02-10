#pragma once

// --- ImGui (needed for ImVec2, ImGui::GetTime) ---
#include "imgui.h"

// --- Editor components ---
#include "TextEditor.h"
#include "TextASTSync.h"
#include "SyntaxHighlighter.h"
#include "KeybindingManager.h"
#include "BufferManager.h"
#include "CodeEditorWidget.h"
#include "EditorMode.h"
#include "FileDialog.h"
#include "FileTree.h"
#include "WelcomeScreen.h"
#include "DragDropHandler.h"
#include "FileWatcher.h"
#include "LSPClient.h"
#include "Pipeline.h"
#include "Diagnostics.h"
#include "SettingsManager.h"
#include "LayoutManager.h"
#include "ASTMutationAPI.h"
#include "MemoryStrategyInference.h"
#include "AnnotationConflict.h"
#include "StrategyDashboard.h"
#include "TransformEngine.h"
#include "StrategyAwareOptimizer.h"
#include "CrossLanguageProjector.h"
#include "DiffUtils.h"
#include "EditorModePolicy.h"
#include "RefactorActions.h"
#include "CommandPalette.h"
#include "ContextAPI.h"
#include "Breadcrumbs.h"
#include "ProjectSearch.h"
#include "GoToLine.h"
#include "Orchestrator.h"
#include "ProjectManager.h"
#include "SessionManager.h"
#include "ZoomUtils.h"
#include "TerminalPanel.h"
#include "WebSocketServer.h"
#include "WorkflowRecorder.h"
#include "BuildSystem.h"
#include "DependencyPanel.h"
#include "LibraryIndexer.h"
#include "LibraryBrowserPanel.h"
#include "ImportManager.h"
#include "PrimitivesRegistry.h"
#include "AgentLibraryPolicy.h"
#include "AgentCodeGen.h"
#include "AgentAnnotationAssistant.h"
#include "AgentPermissionPolicy.h"
#include "CompositionPanel.h"
#include "IncrementalOptimizer.h"
#include "EmacsIntegration.h"
#include "EmacsPackageBrowser.h"
#include "EmacsFunctionDiscovery.h"
#include "EmacsKeybinding.h"
#include "OrgMode.h"
#include "ast/Serialization.h"
#include "ast/Generator.h"
#include "ast/Annotation.h"

// --- Standard library ---
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cctype>
#include <climits>
#include <filesystem>
#include <map>
#include <memory>
#include <cstdlib>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
//  Forward declarations for utility functions defined in EditorUtils.h
//  (used inside EditorState inline method bodies)
// ---------------------------------------------------------------------------
static std::string generateForLanguage(const Module* ast, const std::string& language);
static std::unique_ptr<Module> cloneModule(const Module* ast);
static ASTNode* findNodeAtPosition(ASTNode* node, int line, int col);
static ASTNode* findNodeById(ASTNode* node, const std::string& id);
static int countAnnotationNodes(const ASTNode* node);

// ---------------------------------------------------------------------------
//  Simple helpers used by EditorState methods
// ---------------------------------------------------------------------------
static inline std::string bufferModeToString(BufferManager::BufferMode mode) {
    return mode == BufferManager::BufferMode::Text ? "text" : "structured";
}

static inline BufferManager::BufferMode bufferModeFromString(const std::string& mode) {
    if (mode == "text") return BufferManager::BufferMode::Text;
    return BufferManager::BufferMode::Structured;
}

// ---------------------------------------------------------------------------
//  Editor application state
// ---------------------------------------------------------------------------
struct BufferState {
    TextEditor        editor;
    TextASTSync       sync;
    CodeEditorWidget  widget;
    CodeEditorWidget  generatedWidget;
    EditorMode        mode;
    EditorMode        generatedMode;
    std::string       language    = "python";
    std::string       generatedLanguage = "python";
    std::string       path        = "(untitled)";
    bool              readOnly    = false;
    BufferManager::BufferMode bufferMode = BufferManager::BufferMode::Structured;
    bool              modified    = false;
    int               cursorLine  = 1;
    int               cursorCol   = 1;
    int               lspVersion  = 1;
    std::string       editBuf;
    std::string       generatedBuf;
    std::vector<HighlightSpan> highlights;
    std::vector<HighlightSpan> generatedHighlights;
    bool              highlightsDirty = true;
    bool              generatedHighlightsDirty = true;
    int               generatedHighlightLine = -1;
    float             splitScrollX = 0.0f;
    float             splitScrollY = 0.0f;
    IncrementalOptimizer incrementalOptimizer;
    Orchestrator      orchestrator;
    bool              orchestratorDirty = true;
    int               undoDepth = 0;
};

struct EditorState {
    KeybindingManager keys;
    BufferManager     buffers;
    std::map<std::string, std::unique_ptr<BufferState>> bufferStates;
    BufferState*      activeBuffer = nullptr;
    EmacsConnection   emacs;

    // Find/Replace state
    bool              showFind    = false;
    char              findBuf[256]    = {};
    char              replaceBuf[256] = {};
    int               lastFindPos     = 0;
    bool              showProjectSearch = false;
    char              searchQuery[256] = {};
    char              searchInclude[256] = {};
    char              searchExclude[256] = {};
    bool              searchUseRegex = true;
    std::vector<ProjectSearch::FileResult> searchResults;
    bool              showGoToLine = false;
    char              goToLineBuf[64] = {};
    bool              goToLineError = false;

    // Bottom panel
    int               bottomTab   = 0;  // 0=Output, 1=AST, 2=Highlighted
    std::string       outputLog;
    bool              showTerminalPanel = false;
    TerminalPanel     terminal;
    bool              runInProgress = false;
    bool              hasRunResult = false;
    int               lastRunExitCode = 0;
    std::string       lastRunCommand;
    std::unique_ptr<WebSocketAgentServer> agentServer;
    MockWebSocketTransport* agentTransport = nullptr;
    int               agentPort = 8765;
    std::vector<std::string> agentLog;
    std::map<std::string, AgentRole> agentRoles;
    WorkflowRecorder workflowRecorder;
    BuildSystem::Type buildType = BuildSystem::Type::None;
    std::vector<BuildError> buildErrors;
    std::string lastBuildOutput;
    std::string lastBuildCommand;
    bool              showDependencyPanel = true;
    DependencyPanelState dependencyPanel;
    bool              showLibraryBrowserPanel = true;
    LibraryBrowserState libraryBrowser;
    bool              showCompositionPanel = false;
    CompositionPanelState compositionPanel;
    bool              showEmacsPackagesPanel = false;
    EmacsPackageBrowserState emacsPackages;
    EmacsFunctionIndex emacsFunctionIndex;
    bool              emacsFunctionIndexDirty = true;
    EmacsKeybindingState emacsKeys;
    bool              showEmacsBridgePanel = false;
    bool              showOrgPanel = true;
    OrgDocumentState  orgDoc;
    int               orgTempCounter = 0;
    struct LibraryIndexRequest {
        std::string name;
        std::string version;
        std::string source;
        int symbolsRequestId = -1;
        int completionRequestId = -1;
    };
    std::vector<LibraryIndexRequest> libraryIndexRequests;
    LibraryIndexData libraryIndex;
    PrimitivesRegistry primitives;

    // Custom editor widget state
    bool              showWhitespace = false;
    bool              showMinimap = false;
    bool              showAnnotations = false;
    bool              showOutline = true;
    bool              showLineNumbers = true;
    char              outlineFilter[128] = {};
    LayoutPreset      layoutPreset = LayoutPreset::VSCode;
    WelcomeScreen     welcome;
    std::string       workspaceRoot;
    FileTree          fileTree;
    FileNode          fileTreeRoot;
    bool              fileTreeDirty = true;
    FileWatcher       watcher;
    ProjectSearch     projectSearch;
    std::shared_ptr<LSPTransport> lspTransport;
    std::shared_ptr<LSPClient> lsp;
    bool              symbolsPending = false;
    double            symbolsLastChange = 0.0;
    bool              completionPending = false;
    bool              completionVisible = false;
    int               completionSelected = 0;
    double            completionLastChange = 0.0;
    bool              hoverPending = false;
    double            hoverLastMove = 0.0;
    ImVec2            hoverLastMouse = ImVec2(-1.0f, -1.0f);
    bool              definitionPending = false;
    double            definitionLastRequest = 0.0;
    int               definitionRequestLine = 0;
    int               definitionRequestCol = 0;
    std::string       definitionRequestPath;
    LSPClient::DefinitionLocation definitionPreview;
    bool              definitionPreviewValid = false;
    Pipeline          pipeline;
    bool              analysisPending = false;
    double            analysisLastChange = 0.0;
    std::vector<EditorDiagnostic> whetstoneDiagnostics;
    std::vector<EditorDiagnostic> emacsDiagnostics;
    SettingsManager   settings;
    bool              showLspSettings = false;
    bool              showSettingsPanel = false;
    double            lastAutoSave = 0.0;
    ASTMutationAPI    mutator;
    std::vector<MemoryStrategyInference::Suggestion> suggestions;
    bool              showSuggestionPopup = false;
    MemoryStrategyInference::Suggestion suggestionPopup;
    std::string       optimizeFoldSummary;
    std::string       optimizeDeadCodeSummary;
    std::string       optimizeApplySummary;
    bool              optimizePreview = false;
    bool              showRefactorPopup = false;
    int               refactorAction = 0; // 1=rename,2=extract,3=inline
    char              refactorNameA[128] = {};
    char              refactorNameB[128] = {};
    std::string       refactorError;
    CommandPalette    commandPalette;
    std::unordered_map<std::string, std::function<void()>> commandHandlers;
    bool              showCommandPalette = false;
    char              commandQuery[128] = {};
    int               commandSelected = 0;
    struct DiffState {
        bool active = false;
        bool preview = false;
        int action = 0; // 1=constant-fold, 2=dead-code-elim, 3=apply-all
        bool batch = false;
        std::string beforeText;
        std::string afterText;
        std::vector<std::string> transformIds;
        std::vector<BatchMutationAPI::Mutation> batchMutations;
        std::vector<int> beforeLines;
        std::vector<int> afterLines;
    } diff;
    CodeEditorWidget  diffLeftWidget;
    CodeEditorWidget  diffRightWidget;
    float             diffScrollX = 0.0f;
    float             diffScrollY = 0.0f;

    BufferState* active() { return activeBuffer; }
    bool isStructured() const {
        return activeBuffer && allowStructuredFeatures(activeBuffer->bufferMode);
    }
    Module* activeAST() {
        if (!activeBuffer) return nullptr;
        if (!allowStructuredFeatures(activeBuffer->bufferMode)) return nullptr;
        return activeBuffer->sync.getAST();
    }

    static std::string toFileUri(const std::string& path) {
        std::filesystem::path p(path);
        return "file:///" + p.generic_string();
    }

    static std::string fromFileUri(const std::string& uri) {
        const std::string prefix = "file:///";
        if (uri.rfind(prefix, 0) != 0) return uri;
        std::string path = uri.substr(prefix.size());
        std::replace(path.begin(), path.end(), '/', '\\');
        return path;
    }

    static int byteOffsetForLineCol(const std::string& text, int lineZero, int colZero) {
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

    static int wordStartAt(const std::string& text, int pos) {
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

    static bool isWordChar(char c) {
        return std::isalnum((unsigned char)c) || c == '_';
    }

    static std::string wordAt(const std::string& text, int pos) {
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

    static std::string wordAtLineCol(const std::string& text, int lineZero, int colZero) {
        int pos = byteOffsetForLineCol(text, lineZero, colZero);
        return wordAt(text, pos);
    }

    static std::string wordPrefixAt(const std::string& text, int pos) {
        int start = wordStartAt(text, pos);
        return text.substr(start, pos - start);
    }

    void jumpTo(BufferState* buf, int lineZero, int colZero) {
        if (!buf) return;
        int pos = byteOffsetForLineCol(buf->editBuf, lineZero, colZero);
        buf->widget.setCursor(pos);
        buf->cursorLine = lineZero + 1;
        buf->cursorCol = colZero + 1;
    }

    void applyCompletion(BufferState* buf, const std::string& insertText, int replaceStart, int replaceEnd) {
        if (!buf) return;
        replaceStart = std::max(0, std::min(replaceStart, (int)buf->editBuf.size()));
        replaceEnd = std::max(replaceStart, std::min(replaceEnd, (int)buf->editBuf.size()));
        buf->editBuf.replace(replaceStart, replaceEnd - replaceStart, insertText);
        int newPos = replaceStart + (int)insertText.size();
        buf->widget.setCursor(newPos);
        activeBuffer = buf;
        onTextChanged();
    }

    void insertTextAtCursor(const std::string& text) {
        if (!active() || text.empty()) return;
        int cursor = active()->widget.getCursor();
        cursor = std::max(0, std::min(cursor, (int)active()->editBuf.size()));
        active()->editBuf.insert(cursor, text);
        active()->widget.setCursor(cursor + (int)text.size());
        onTextChanged();
    }

    void ensureImportForSymbol(const std::string& library, const std::string& symbol) {
        if (!active() || library.empty()) return;
        std::string clean = symbol;
        auto paren = clean.find('(');
        if (paren != std::string::npos) clean = clean.substr(0, paren);
        ImportEditResult result = ensureImport(active()->editBuf,
                                               active()->language,
                                               library,
                                               clean);
        if (result.changed) {
            active()->editBuf = result.text;
            onTextChanged();
        }
    }

    void appendUnusedImportDiagnostics(std::vector<EditorDiagnostic>& diags) {
        if (!active()) return;
        auto issues = findUnusedImports(active()->editBuf, active()->language);
        std::string uri = toFileUri(active()->path);
        for (const auto& issue : issues) {
            EditorDiagnostic ed;
            ed.uri = uri;
            ed.line = issue.line;
            ed.character = 0;
            ed.severity = 2;
            ed.message = "[Imports] " + issue.message;
            ed.source = "ImportManager";
            diags.push_back(std::move(ed));
        }
    }

    std::string makeUntitledName() const {
        if (!buffers.hasBuffer("(untitled)")) return "(untitled)";
        int i = 1;
        while (true) {
            std::string name = "(untitled-" + std::to_string(i) + ")";
            if (!buffers.hasBuffer(name)) return name;
            ++i;
        }
    }

    void createBuffer(const std::string& path, const std::string& content,
                      const std::string& language,
                      BufferManager::BufferMode mode = BufferManager::BufferMode::Structured) {
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
        state->mode.setLanguage(language);
        state->generatedMode.setLanguage(language);
        state->editor.setContent(content, language);
        state->sync.setText(content, language);
        state->sync.syncNow();
        state->incrementalOptimizer.setRoot(state->sync.getAST());
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
        primitives.setRoot(activeAST());
        primitives.setLanguage(active()->language);
        recordUndoSnapshot();
        if (language == "elisp") {
            emacsFunctionIndexDirty = true;
        }
        if (path.rfind("(untitled", 0) != 0) watcher.watch(path);
        if (lsp && path.rfind("(untitled", 0) != 0) {
            lsp->didOpen(toFileUri(path), language, content, activeBuffer->lspVersion);
        }
        symbolsPending = true;
        symbolsLastChange = ImGui::GetTime();
        if (lsp) lsp->clearDocumentSymbols();
    }

    bool jumpToDefinitionLocation(const LSPClient::DefinitionLocation& loc) {
        if (loc.uri.empty()) return false;
        std::string path = fromFileUri(loc.uri);
        if (path.empty()) return false;
        if (buffers.hasBuffer(path)) switchToBuffer(path);
        else doOpen(path);
        jumpTo(active(), loc.range.start.line, loc.range.start.character);
        return true;
    }

    bool goToDefinitionFallback(int lineZero, int colZero) {
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

    bool goToDefinitionAt(int lineZero, int colZero) {
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

    bool previewDefinitionAt(int lineZero, int colZero, std::string& outLabel) {
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

    void switchToBuffer(const std::string& path) {
        if (!buffers.hasBuffer(path)) return;
        buffers.switchToBuffer(path);
        activeBuffer = bufferStates[path].get();
        if (active()) active()->orchestratorDirty = true;
        primitives.setRoot(activeAST());
        primitives.setLanguage(active()->language);
        symbolsPending = true;
        symbolsLastChange = ImGui::GetTime();
        if (lsp) lsp->clearDocumentSymbols();
        if (active() && active()->language == "elisp") {
            emacsFunctionIndexDirty = true;
        }
    }

    std::filesystem::path configDir() const {
        const char* home = std::getenv("USERPROFILE");
        if (!home) home = std::getenv("HOME");
        std::filesystem::path base = home ? std::filesystem::path(home) : std::filesystem::current_path();
        return base / ".whetstone";
    }

    std::filesystem::path recentFilePath() const {
        return configDir() / "recent.json";
    }

    void loadRecentFiles() {
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

    void saveRecentFiles() {
        std::filesystem::path p = recentFilePath();
        std::filesystem::create_directories(p.parent_path());
        nlohmann::json j = nlohmann::json::array();
        for (const auto& rf : welcome.getRecentFiles()) {
            j.push_back({{"path", rf.path}, {"language", rf.language}, {"mode", rf.mode}});
        }
        std::ofstream out(p.string(), std::ios::binary);
        out << j.dump(2);
    }

    void closeAllBuffers() {
        auto open = buffers.getOpenBuffers();
        for (const auto& path : open) {
            buffers.closeBuffer(path);
            bufferStates.erase(path);
            if (path.rfind("(untitled", 0) != 0) watcher.unwatch(path);
        }
        activeBuffer = nullptr;
    }

    bool saveProject(const std::string& path) {
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

    bool loadProject(const std::string& path) {
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
                projectSearch.setRoot(workspaceRoot);
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

    std::filesystem::path sessionFilePath() const {
        return configDir() / "session.json";
    }

    std::filesystem::path settingsFilePath() const {
        return configDir() / "settings.json";
    }

    bool saveSession(const std::string& imguiIni) {
        SessionData session;
        session.workspaceRoot = workspaceRoot;
        session.activePath = active() ? active()->path : "";
        session.layoutPreset = LayoutManager::presetName(layoutPreset);
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

    bool loadSession(SessionData& out) {
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

    void applySession(const SessionData& session) {
        closeAllBuffers();
        if (!session.workspaceRoot.empty()) {
            workspaceRoot = session.workspaceRoot;
            fileTreeDirty = true;
            projectSearch.setRoot(workspaceRoot);
        }
        layoutPreset = LayoutManager::presetFromName(session.layoutPreset);
        for (const auto& buf : session.buffers) {
            doOpen(buf.path, bufferModeFromString(buf.mode));
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

    void applyTabSizeToBuffers(int size) {
        for (auto& kv : bufferStates) {
            kv.second->mode.setTabSize(size);
            kv.second->generatedMode.setTabSize(size);
        }
    }

    void applySettingsToState() {
        showMinimap = settings.getShowMinimap();
        showLineNumbers = settings.getShowLineNumbers();
        layoutPreset = LayoutManager::presetFromName(settings.getLayoutPreset());
        keys.setProfile(KeybindingManager::profileFromName(settings.getKeybindingProfile()));
        registerCommands();
        applyTabSizeToBuffers(settings.getTabSize());
    }

    void loadSettingsFromDisk() {
        settings.loadFromFile(settingsFilePath().string());
        applySettingsToState();
    }

    void saveSettingsToDisk() {
        settings.setShowMinimap(showMinimap);
        settings.setShowLineNumbers(showLineNumbers);
        settings.setLayoutPreset(LayoutManager::presetName(layoutPreset));
        settings.setKeybindingProfile(KeybindingManager::profileName(keys.getProfile()));
        std::filesystem::create_directories(settingsFilePath().parent_path());
        settings.saveToFile(settingsFilePath().string());
    }

    void init() {
        outputLog = "Whetstone Editor ready.\n";
        workspaceRoot = std::filesystem::current_path().string();
        projectSearch.setRoot(workspaceRoot);
        fileTreeDirty = true;
        loadRecentFiles();
        loadSettingsFromDisk();
        registerCommands();
        startEmacsDaemonFromSettings();
        initAgentServer();
        refreshBuildSystem();
    }

    std::string orgTempExtension(const std::string& language) const {
        if (language == "python") return ".py";
        if (language == "cpp") return ".cpp";
        if (language == "elisp") return ".el";
        if (language == "javascript") return ".js";
        if (language == "typescript") return ".ts";
        if (language == "java") return ".java";
        if (language == "rust") return ".rs";
        if (language == "go") return ".go";
        return ".txt";
    }

    std::string writeOrgTempFile(const std::string& language, const std::string& code) {
        std::filesystem::path root = workspaceRoot.empty()
            ? std::filesystem::current_path()
            : std::filesystem::path(workspaceRoot);
        std::filesystem::path dir = root / ".whetstone_org";
        std::filesystem::create_directories(dir);
        std::string name = "org_block_" + std::to_string(++orgTempCounter) + orgTempExtension(language);
        std::filesystem::path filePath = dir / name;
        std::ofstream out(filePath.string(), std::ios::binary);
        out << code;
        return filePath.string();
    }

    std::string runOrgBlock(const std::string& language, const std::string& code) {
        if (language == "elisp") {
            std::string result = emacs.sendCommand(ElispCommandBuilder::eval(code));
            if (!emacs.getLastError().empty() && result == "error") {
                return emacs.getLastError();
            }
            return result.empty() ? "OK" : result;
        }
        std::string path = writeOrgTempFile(language, code);
        std::string cmd = buildRunCommand(path, language, false);
        if (cmd.empty()) return "No runner for language: " + language;
        showTerminalPanel = true;
        std::string output;
        int codeExit = terminal.runAndCapture(workspaceRoot, cmd, output);
        output += "[exit code: " + std::to_string(codeExit) + "]";
        return output;
    }

    void startEmacsDaemonFromSettings() {
        emacsDiagnostics.clear();
        std::string initPath = resolveEmacsInitPath();
        std::string log;
        bool ok = emacs.startDaemonWithConfig(initPath, log);
        if (!log.empty()) {
            outputLog += "[emacs] " + log + "\n";
        }
        if (!ok) {
            EditorDiagnostic d;
            d.uri = "emacs://init";
            d.severity = 1;
            d.message = "[Emacs] " + emacs.getLastError();
            d.source = "EmacsDaemon";
            emacsDiagnostics.push_back(d);
            return;
        }
        if (!log.empty()) {
            addEmacsLogDiagnostics(log);
        }
    }

    std::string resolveEmacsInitPath() const {
        return ::resolveEmacsInitPath(settings.getEmacsConfigPath());
    }

    void addEmacsLogDiagnostics(const std::string& log) {
        std::istringstream ss(log);
        std::string line;
        while (std::getline(ss, line)) {
            std::string lower = line;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return (char)std::tolower(c); });
            if (lower.find("error") != std::string::npos) {
                EditorDiagnostic d;
                d.uri = "emacs://init";
                d.severity = 1;
                d.message = "[Emacs] " + line;
                d.source = "EmacsDaemon";
                emacsDiagnostics.push_back(d);
            }
        }
    }

    void initAgentServer() {
        auto transport = std::make_unique<MockWebSocketTransport>();
        agentTransport = transport.get();
        agentServer = std::make_unique<WebSocketAgentServer>(std::move(transport));
        agentServer->setRequestHandler([this](const json& request,
                                              const std::string& sessionId) {
            return processAgentRequest(request, sessionId);
        });
        agentServer->setSessionEventCallback([this](const AgentSession& s,
                                                   const std::string& event) {
            logAgentEvent("Agent " + s.sessionId + " " + event);
            if (event == "connected") {
                agentRoles[s.sessionId] = AgentRole::Linter;
            } else if (event == "disconnected") {
                agentRoles.erase(s.sessionId);
            }
        });
        agentServer->setRequestLogCallback([this](const std::string& sid,
                                                  const json& req,
                                                  const json& res) {
            std::string method = req.value("method", "");
            bool ok = !res.contains("error");
            logAgentEvent("RPC " + sid + " " + method + (ok ? " ok" : " error"));
            workflowRecorder.record(sid, req, res);
        });
        if (agentServer->start(agentPort)) {
            logAgentEvent("Agent server started on port " + std::to_string(agentPort));
        } else {
            logAgentEvent("Agent server failed to start on port " + std::to_string(agentPort));
        }
    }

    void shutdownAgentServer() {
        if (agentServer) agentServer->stop();
    }

    void logAgentEvent(const std::string& msg) {
        agentLog.push_back(msg);
    }

    AgentRole getAgentRole(const std::string& sessionId) const {
        auto it = agentRoles.find(sessionId);
        return it != agentRoles.end() ? it->second : AgentRole::Linter;
    }

    void setAgentRole(const std::string& sessionId, AgentRole role) {
        agentRoles[sessionId] = role;
    }

    std::string agentActorLabel(const std::string& sessionId) const {
        std::string label = "agent:" + sessionId;
        if (agentServer) {
            if (const auto* sess = agentServer->getSession(sessionId)) {
                if (!sess->agentName.empty()) {
                    label = "agent:" + sess->agentName;
                }
            }
        }
        return label;
    }

    void refreshBuildSystem() {
        buildType = BuildSystem::detect(workspaceRoot);
    }

    int runBuildCommand(const BuildCommand& cmd) {
        if (workspaceRoot.empty()) {
            terminal.append("[build] No workspace root set.\n");
            return -1;
        }
        showTerminalPanel = true;
        lastBuildCommand = cmd.command;
        int code = terminal.runAndCapture(workspaceRoot, cmd.command, lastBuildOutput);
        buildErrors = BuildSystem::parseErrors(lastBuildOutput);
        outputLog += "[build] " + cmd.label + " => exit " + std::to_string(code) + "\n";
        return code;
    }

    void pollLspMessages() {
        if (!lsp || !lspTransport || !lspTransport->isOpen()) return;
        std::string msg;
        while (lspTransport->receive(msg)) {
            lsp->handleMessage(msg);
        }
    }

    void requestLibraryIndex() {
        libraryIndex.symbolsByLibrary.clear();
        libraryIndex.completionsByLibrary.clear();
        libraryIndexRequests.clear();
        if (!isStructured() || !activeAST()) {
            outputLog += "[deps] Library index skipped: no structured AST.\n";
            return;
        }
        if (!lsp || !lspTransport || !lspTransport->isOpen()) {
            applyStubFallback(libraryIndex, dependencyPanel.deps, workspaceRoot);
            rebuildExternalModulesFromIndex();
            outputLog += "[deps] Library index uses cached symbols (LSP offline).\n";
            return;
        }

        for (const auto& dep : dependencyPanel.deps) {
            if (dep.name.empty()) continue;
            LibraryIndexRequest req;
            req.name = dep.name;
            req.version = dep.version;
            req.source = dep.source;
            req.symbolsRequestId = lsp->requestWorkspaceSymbols(dep.name);
            if (active() && active()->path.rfind("(untitled", 0) != 0) {
                int line = std::max(0, active()->cursorLine - 1);
                int col = std::max(0, active()->cursorCol - 1);
                req.completionRequestId = lsp->requestLibraryCompletion(
                    toFileUri(active()->path), line, col, dep.name + ".");
            }
            libraryIndexRequests.push_back(req);
        }
        applyStubFallback(libraryIndex, dependencyPanel.deps, workspaceRoot);
        rebuildExternalModulesFromIndex();
        outputLog += "[deps] Library index requests queued: " +
                     std::to_string(libraryIndexRequests.size()) + "\n";
    }

    void processLibraryIndexResponses() {
        if (!lsp) return;
        bool updated = false;
        for (auto& req : libraryIndexRequests) {
            if (req.symbolsRequestId >= 0) {
                std::vector<LSPClient::WorkspaceSymbol> symbols;
                if (lsp->takeWorkspaceSymbols(req.symbolsRequestId, symbols)) {
                    libraryIndex.symbolsByLibrary[req.name] = std::move(symbols);
                    req.symbolsRequestId = -1;
                    updated = true;
                }
            }
            if (req.completionRequestId >= 0) {
                std::vector<LSPClient::CompletionItem> items;
                if (lsp->takeLibraryCompletions(req.completionRequestId, items)) {
                    std::vector<std::string> labels;
                    labels.reserve(items.size());
                    for (const auto& item : items) {
                        if (!item.label.empty()) labels.push_back(item.label);
                    }
                    if (!labels.empty()) {
                        libraryIndex.completionsByLibrary[req.name] = std::move(labels);
                    }
                    req.completionRequestId = -1;
                    updated = true;
                }
            }
        }
        if (updated) {
            applyStubFallback(libraryIndex, dependencyPanel.deps, workspaceRoot);
            rebuildExternalModulesFromIndex();
            outputLog += "[deps] Library index updated.\n";
        }
    }

    void updateEmacsFunctionIndex() {
        if (!active() || active()->language != "elisp") return;
        std::string error;
        auto packages = queryEmacsPackageList(emacs, false, outputLog, error);
        if (!error.empty()) return;
        refreshEmacsFunctionIndex(emacsFunctionIndex, emacs, packages, outputLog);
        addEmacsSymbolsToLibraryIndex(libraryIndex, emacsFunctionIndex);
        emacsFunctionIndexDirty = false;
        rebuildExternalModulesFromIndex();
        outputLog += "[emacs] Indexed functions for " +
                     std::to_string(emacsFunctionIndex.functionsByPackage.size()) +
                     " packages.\n";
    }

    void rebuildExternalModulesFromIndex() {
        Module* ast = activeAST();
        if (!ast) return;
        rebuildExternalModules(ast, dependencyPanel.deps, libraryIndex);
        if (active() && active()->language == "elisp") {
            int signatureId = 0;
            appendEmacsExternalModules(ast, emacsFunctionIndex, signatureId);
        }
        if (active()) active()->orchestratorDirty = true;
    }

    json buildDiagnosticsJson() const {
        json out;
        json lspArr = json::array();
        auto diags = lsp ? lsp->getDiagnostics() : std::vector<LSPClient::Diagnostic>{};
        for (const auto& d : diags) {
            lspArr.push_back({
                {"uri", d.uri},
                {"message", d.message},
                {"severity", d.severity},
                {"range", {
                    {"start", {{"line", d.range.start.line}, {"character", d.range.start.character}}},
                    {"end", {{"line", d.range.end.line}, {"character", d.range.end.character}}}
                }}
            });
        }

        json whetArr = json::array();
        for (const auto& d : whetstoneDiagnostics) {
            whetArr.push_back({
                {"uri", d.uri},
                {"message", d.message},
                {"severity", d.severity},
                {"line", d.line},
                {"character", d.character}
            });
        }

        json emacsArr = json::array();
        for (const auto& d : emacsDiagnostics) {
            emacsArr.push_back({
                {"uri", d.uri},
                {"message", d.message},
                {"severity", d.severity},
                {"line", d.line},
                {"character", d.character}
            });
        }

        out["lsp"] = lspArr;
        out["whetstone"] = whetArr;
        out["emacs"] = emacsArr;
        return out;
    }

    json processAgentRequest(const json& request, const std::string& sessionId) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        std::string method = request.value("method", "");
        AgentRole role = getAgentRole(sessionId);
        if (method == "getAST") {
            if (!active() || !isStructured()) {
                response["error"] = {{"code", -32000}, {"message", "No structured buffer"}};
                return response;
            }
            Module* ast = activeAST();
            if (!ast) {
                response["error"] = {{"code", -32001}, {"message", "AST unavailable"}};
                return response;
            }
            response["result"] = {
                {"ast", toJson(ast)},
                {"annotationCount", countAnnotationNodes(ast)},
                {"diagnostics", buildDiagnosticsJson()}
            };
            return response;
        }

        if (method == "generateCode") {
            if (!AgentPermissionPolicy::canInvoke(role, method)) {
                response["error"] = {{"code", -32031}, {"message", "Role not permitted"}};
                return response;
            }
            if (!active() || !isStructured()) {
                response["error"] = {{"code", -32000}, {"message", "No structured buffer"}};
                return response;
            }
            Module* ast = activeAST();
            if (!ast) {
                response["error"] = {{"code", -32001}, {"message", "AST unavailable"}};
                return response;
            }
            auto params = request.contains("params") ? request["params"] : json::object();
            std::string spec = params.value("spec", "");
            bool preferImports = params.value("preferImports", true);

            primitives.setRoot(ast);
            primitives.setLanguage(active()->language);

            AgentCodeGen gen;
            AgentCodeGenResult genRes = gen.generate(spec, primitives, active()->language, preferImports);
            if (!genRes.node) {
                response["error"] = {{"code", -32020}, {"message", "Code generation failed"}};
                return response;
            }

            json nodeJson = toJson(genRes.node);
            deleteTree(genRes.node);

            response["result"] = {
                {"node", nodeJson},
                {"note", genRes.note},
                {"usedSymbols", genRes.usedSymbols},
                {"language", active()->language}
            };
            return response;
        }

        if (method == "setAgentRole") {
            auto params = request.contains("params") ? request["params"] : json::object();
            std::string roleName = params.value("role", "linter");
            AgentRole newRole = AgentPermissionPolicy::roleFromString(roleName);
            setAgentRole(sessionId, newRole);
            response["result"] = {
                {"role", AgentPermissionPolicy::roleLabel(newRole)}
            };
            return response;
        }

        if (method == "startWorkflowRecording") {
            auto params = request.contains("params") ? request["params"] : json::object();
            std::string name = params.value("name", "workflow");
            workflowRecorder.startRecording(name, sessionId);
            response["result"] = {
                {"recording", true},
                {"name", name}
            };
            return response;
        }

        if (method == "stopWorkflowRecording") {
            response["result"] = workflowRecorder.stopRecording();
            return response;
        }

        if (method == "getWorkflowRecording") {
            response["result"] = workflowRecorder.exportWorkflow();
            return response;
        }

        if (method == "replayWorkflow") {
            auto params = request.contains("params") ? request["params"] : json::object();
            if (!params.contains("workflow")) {
                response["error"] = {{"code", -32602}, {"message", "Missing workflow payload"}};
                return response;
            }
            WorkflowRecorder temp;
            if (!temp.loadWorkflow(params["workflow"])) {
                response["error"] = {{"code", -32602}, {"message", "Invalid workflow payload"}};
                return response;
            }
            auto requests = temp.buildReplayRequests();
            json results = json::array();
            workflowRecorder.setReplaying(true);
            for (auto& req : requests) {
                results.push_back(processAgentRequest(req, sessionId));
            }
            workflowRecorder.setReplaying(false);
            response["result"] = {
                {"count", results.size()},
                {"responses", results}
            };
            return response;
        }

        if (method == "getAnnotationSuggestions") {
            if (!AgentPermissionPolicy::canInvoke(role, method)) {
                response["error"] = {{"code", -32031}, {"message", "Role not permitted"}};
                return response;
            }
            if (!active() || !isStructured()) {
                response["error"] = {{"code", -32000}, {"message", "No structured buffer"}};
                return response;
            }
            Module* ast = activeAST();
            if (!ast) {
                response["error"] = {{"code", -32001}, {"message", "AST unavailable"}};
                return response;
            }

            auto params = request.contains("params") ? request["params"] : json::object();
            std::string nodeId = params.value("nodeId", "");
            int line = params.value("line", -1);
            int col = params.value("col", -1);

            if (!nodeId.empty()) {
                if (!findNodeById(ast, nodeId)) {
                    response["error"] = {{"code", -32002}, {"message", "Node not found: " + nodeId}};
                    return response;
                }
            } else if (line >= 0 && col >= 0) {
                ASTNode* posNode = findNodeAtPosition(ast, line, col);
                if (posNode) nodeId = posNode->id;
            }

            AgentAnnotationAssistant assistant;
            auto result = assistant.suggest(ast, nodeId);

            json suggArr = json::array();
            for (const auto& s : result.suggestions) {
                suggArr.push_back({
                    {"nodeId", s.nodeId},
                    {"annotationType", s.annotationType},
                    {"strategy", s.strategy},
                    {"reason", s.reason},
                    {"confidence", s.confidence}
                });
            }

            json diagArr = json::array();
            for (const auto& d : result.diagnostics) {
                diagArr.push_back({
                    {"severity", d.severity},
                    {"message", d.message},
                    {"nodeId", d.nodeId}
                });
            }

            response["result"] = {
                {"scopeId", result.scopeNodeId},
                {"suggestions", suggArr},
                {"diagnostics", diagArr}
            };
            return response;
        }

        if (method == "applyAnnotationSuggestion") {
            if (!AgentPermissionPolicy::canInvoke(role, method)) {
                response["error"] = {{"code", -32031}, {"message", "Role not permitted"}};
                return response;
            }
            if (!active() || !isStructured()) {
                response["error"] = {{"code", -32000}, {"message", "No structured buffer"}};
                return response;
            }
            Module* ast = mutationAST();
            if (!ast) {
                response["error"] = {{"code", -32001}, {"message", "AST unavailable"}};
                return response;
            }

            auto params = request.contains("params") ? request["params"] : json::object();
            MemoryStrategyInference::Suggestion suggestion;
            suggestion.nodeId = params.value("nodeId", "");
            suggestion.annotationType = params.value("annotationType", "");
            suggestion.strategy = params.value("strategy", "");
            suggestion.reason = params.value("reason", "");
            suggestion.confidence = params.value("confidence", 0.0);

            AgentAnnotationAssistant assistant;
            auto res = assistant.applySuggestion(ast, suggestion);
            if (!res.success) {
                response["error"] = {{"code", -32010}, {"message", res.error}};
                return response;
            }

            if (params.contains("accepted")) {
                bool accepted = params.value("accepted", true);
                assistant.recordFeedback(suggestion, accepted);
            }

            applyOrchestratorToActive();
            if (active()) {
                std::vector<std::string> affected;
                if (!suggestion.nodeId.empty()) affected.push_back(suggestion.nodeId);
                if (!suggestion.annotationType.empty() && !suggestion.nodeId.empty()) {
                    affected.push_back("anno_" + suggestion.annotationType + "_" + suggestion.nodeId);
                }
                active()->incrementalOptimizer.setRoot(active()->sync.getAST());
                active()->incrementalOptimizer.recordExternalTransform(
                    "agent-annotation",
                    affected,
                    agentActorLabel(sessionId));
            }
            response["result"] = {
                {"success", true},
                {"warning", res.warning}
            };
            return response;
        }

        if (method == "recordAnnotationFeedback") {
            if (!AgentPermissionPolicy::canInvoke(role, method)) {
                response["error"] = {{"code", -32031}, {"message", "Role not permitted"}};
                return response;
            }
            auto params = request.contains("params") ? request["params"] : json::object();
            MemoryStrategyInference::Suggestion suggestion;
            suggestion.nodeId = params.value("nodeId", "");
            suggestion.annotationType = params.value("annotationType", "");
            suggestion.strategy = params.value("strategy", "");
            suggestion.reason = params.value("reason", "");
            suggestion.confidence = params.value("confidence", 0.0);

            bool accepted = params.value("accepted", false);
            AgentAnnotationAssistant assistant;
            assistant.recordFeedback(suggestion, accepted);
            response["result"] = true;
            return response;
        }

        if (method == "applyMutation") {
            if (!AgentPermissionPolicy::canInvoke(role, method)) {
                response["error"] = {{"code", -32031}, {"message", "Role not permitted"}};
                return response;
            }
            if (!active() || !isStructured()) {
                response["error"] = {{"code", -32000}, {"message", "No structured buffer"}};
                return response;
            }
            auto params = request.contains("params") ? request["params"] : json::object();
            std::string type = params.value("type", "");
            bool preferImports = params.value("preferImports", false);
            bool strictMode = params.value("strictMode", false);
            Module* ast = mutationAST();
            if (!ast) {
                response["error"] = {{"code", -32001}, {"message", "AST unavailable"}};
                return response;
            }
            ASTMutationAPI mut;
            mut.setRoot(ast);
            ASTMutationAPI::MutationResult res;
            LibraryPolicyResult policy;
            std::vector<std::string> affectedIds;

            if (type == "setProperty") {
                std::string nodeId = params.value("nodeId", "");
                if (!nodeId.empty()) affectedIds.push_back(nodeId);
                res = mut.setProperty(params.value("nodeId", ""),
                                      params.value("property", ""),
                                      params.value("value", ""));
            } else if (type == "updateNode") {
                std::map<std::string, std::string> props;
                if (params.contains("properties")) {
                    for (auto& [k, v] : params["properties"].items()) {
                        props[k] = v.get<std::string>();
                    }
                }
                std::string nodeId = params.value("nodeId", "");
                if (!nodeId.empty()) affectedIds.push_back(nodeId);
                res = mut.updateNode(params.value("nodeId", ""), props);
            } else if (type == "deleteNode") {
                std::string nodeId = params.value("nodeId", "");
                if (!nodeId.empty()) affectedIds.push_back(nodeId);
                res = mut.deleteNode(params.value("nodeId", ""));
            } else if (type == "insertNode") {
                ASTNode* node = nullptr;
                std::string insertedId;
                if (params.contains("node")) {
                    node = fromJson(params["node"]);
                    if (node) insertedId = node->id;
                }
                if (node) {
                    policy = checkMutationLibraryPolicy(node, ast, preferImports, strictMode);
                    if (!policy.ok) {
                        response["error"] = {{"code", -32011}, {"message", policy.error}};
                        deleteTree(node);
                        return response;
                    }
                }
                res = mut.insertNode(params.value("parentId", ""),
                                     params.value("role", ""), node);
                if (!res.success && node) {
                    deleteTree(node);
                } else if (res.success && !insertedId.empty()) {
                    affectedIds.push_back(insertedId);
                }
            } else {
                response["error"] = {{"code", -32602}, {"message", "Unknown mutation type"}};
                return response;
            }

            if (!res.success) {
                response["error"] = {{"code", -32010}, {"message", res.error}};
                return response;
            }

            applyOrchestratorToActive();
            if (active()) {
                if (affectedIds.empty() && !params.value("nodeId", "").empty()) {
                    affectedIds.push_back(params.value("nodeId", ""));
                }
                active()->incrementalOptimizer.setRoot(active()->sync.getAST());
                active()->incrementalOptimizer.recordExternalTransform(
                    "agent-mutation:" + type,
                    affectedIds,
                    agentActorLabel(sessionId));
            }
            response["result"] = {
                {"success", true},
                {"warning", res.warning},
                {"libraryWarning", policy.warning},
                {"unknownFunctions", policy.unknownFunctions}
            };
            return response;
        }

        response["error"] = {{"code", -32601}, {"message", "Method not found"}};
        return response;
    }

    void syncOrchestratorFromActive() {
        if (!active()) return;
        if (!isStructured()) return;
        Module* ast = active()->sync.getAST();
        if (!ast) return;
        active()->orchestrator.setAST(cloneModule(ast));
        active()->orchestratorDirty = false;
    }

    void applyOrchestratorToActive() {
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

    Module* mutationAST() {
        if (!active()) return nullptr;
        if (!isStructured()) return nullptr;
        if (active()->orchestratorDirty || !active()->orchestrator.getAST()) {
            syncOrchestratorFromActive();
        }
        return active()->orchestrator.getAST();
    }

    void recordUndoSnapshot() {
        if (!active()) return;
        Module* ast = isStructured() ? active()->sync.getAST() : nullptr;
        active()->orchestrator.recordSnapshot(active()->editBuf, ast);
        active()->undoDepth = active()->orchestrator.getUndoDepth();
    }

    void applySnapshotToActive(const std::string& text, std::unique_ptr<Module> ast) {
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
        active()->generatedHighlightsDirty = true;
        active()->modified = true;
        if (lsp && active()->path.rfind("(untitled", 0) != 0) {
            active()->lspVersion += 1;
            lsp->didChange(toFileUri(active()->path), active()->editBuf, active()->lspVersion);
        }
    }

    void registerCommand(const std::string& id,
                         const std::string& label,
                         const std::string& shortcut,
                         std::function<void()> fn) {
        commandPalette.registerCommand(id, label, shortcut);
        commandHandlers[id] = std::move(fn);
    }

    void registerCommands() {
        commandPalette = CommandPalette();
        commandHandlers.clear();

        registerCommand("file.new", "File: New File",
                        keys.getBinding("file.new").toString(),
                        [this]() {
                            std::string lang = active() ? active()->language : "python";
                            createBuffer(makeUntitledName(), "", lang);
                        });
        registerCommand("file.open", "File: Open...",
                        keys.getBinding("file.open").toString(),
                        [this]() {
                            std::string path = FileDialog::openFile(
                                {"Open File", {"*.py","*.cpp","*.h","*.el","*.js","*.ts","*.java","*.rs","*.go"},
                                 std::string()});
                            if (!path.empty()) doOpen(path);
                        });
        registerCommand("file.save", "File: Save",
                        keys.getBinding("file.save").toString(),
                        [this]() { doSave(); });
        registerCommand("project.open", "Project: Open...",
                        "",
                        [this]() {
                            std::string path = FileDialog::openFile(
                                {"Open Project", {"*.whetstone"}, std::string()});
                            if (!path.empty()) {
                                if (!loadProject(path)) {
                                    outputLog += "Failed to open project: " + path + "\n";
                                }
                            }
                        });
        registerCommand("project.save", "Project: Save...",
                        "",
                        [this]() {
                            std::string path = FileDialog::saveFile(
                                {"Save Project", {"*.whetstone"}, std::string()});
                            if (!path.empty()) {
                                if (!saveProject(path)) {
                                    outputLog += "Failed to save project: " + path + "\n";
                                }
                            }
                        });
        registerCommand("edit.undo", "Edit: Undo",
                        keys.getBinding("edit.undo").toString(),
                        [this]() { doUndo(); });
        registerCommand("edit.redo", "Edit: Redo",
                        keys.getBinding("edit.redo").toString(),
                        [this]() { doRedo(); });
        registerCommand("search.find", "Search: Find/Replace",
                        keys.getBinding("search.find").toString(),
                        [this]() { showFind = !showFind; });
        registerCommand("search.findInFiles", "Search: Find in Files",
                        keys.getBinding("search.findInFiles").toString(),
                        [this]() { showProjectSearch = !showProjectSearch; });
        registerCommand("nav.goToLine", "Navigate: Go to Line",
                        keys.getBinding("nav.goToLine").toString(),
                        [this]() {
                            showGoToLine = true;
                            goToLineBuf[0] = '\0';
                            goToLineError = false;
                        });
        registerCommand("view.whitespace", "View: Toggle Whitespace", "",
                        [this]() { showWhitespace = !showWhitespace; });
        registerCommand("view.minimap", "View: Toggle Minimap", "",
                        [this]() { showMinimap = !showMinimap; });
        registerCommand("view.annotations", "View: Toggle Annotations", "",
                        [this]() { showAnnotations = !showAnnotations; });
        registerCommand("view.outline", "View: Toggle Outline", "",
                        [this]() { showOutline = !showOutline; });
        registerCommand("view.toggleTerminal", "View: Toggle Terminal",
                        keys.getBinding("view.toggleTerminal").toString(),
                        [this]() { showTerminalPanel = !showTerminalPanel; });
        registerCommand("view.lsp", "View: LSP Servers...", "",
                        [this]() { showLspSettings = !showLspSettings; });
        registerCommand("mode.toggle", "Mode: Toggle Text/Structured", "",
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
                        [this]() {
                            refactorAction = 1;
                            showRefactorPopup = true;
                            refactorNameA[0] = '\0';
                            refactorNameB[0] = '\0';
                        });
        registerCommand("refactor.extract", "Refactor: Extract Function", "",
                        [this]() {
                            refactorAction = 2;
                            showRefactorPopup = true;
                            refactorNameA[0] = '\0';
                            refactorNameB[0] = '\0';
                        });
        registerCommand("refactor.inline", "Refactor: Inline Variable", "",
                        [this]() {
                            refactorAction = 3;
                            showRefactorPopup = true;
                            refactorNameA[0] = '\0';
                            refactorNameB[0] = '\0';
                        });
        registerCommand("build.run", "Build: Run", keys.getBinding("build.run").toString(),
                        [this]() { runActiveFile(false); });
        registerCommand("build.build", "Build: Build", keys.getBinding("build.build").toString(),
                        [this]() { runActiveFile(true); });
    }

    void executeCommand(const std::string& id) {
        auto it = commandHandlers.find(id);
        if (it == commandHandlers.end()) return;
        it->second();
        commandPalette.markUsed(id);
    }

    void setLanguage(const std::string& lang) {
        if (!active()) return;
        std::string prevLang = active()->language;
        active()->language = lang;
        active()->mode.setLanguage(lang);
        primitives.setLanguage(lang);
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
            emacsFunctionIndexDirty = true;
        }
        if (lang == "org") {
            active()->bufferMode = BufferManager::BufferMode::Text;
            buffers.setBufferMode(active()->path, active()->bufferMode);
        }
    }

    bool handleEmacsKeyChord(const std::string& chord) {
        if (layoutPreset != LayoutPreset::Emacs) return false;
        return emacsHandleKeySequence(emacsKeys, emacs, chord, outputLog);
    }

    void refreshEmacsModeLine(double nowSeconds) {
        if (layoutPreset != LayoutPreset::Emacs) return;
        updateEmacsModeLine(emacsKeys, emacs, nowSeconds, outputLog);
    }

    void pullFromEmacs() {
        if (!active()) return;
        if (active()->path.rfind("(untitled", 0) == 0) {
            outputLog += "[emacs] Save file before syncing.\n";
            return;
        }
        std::string text = emacs.getBufferText(active()->path);
        if (!emacs.getLastError().empty() && text == "error") {
            outputLog += "[emacs] " + emacs.getLastError() + "\n";
            return;
        }
        active()->editBuf = text;
        onTextChanged();
        outputLog += "[emacs] Pulled buffer from Emacs.\n";
    }

    void pushToEmacs() {
        if (!active()) return;
        if (active()->path.rfind("(untitled", 0) == 0) {
            outputLog += "[emacs] Save file before syncing.\n";
            return;
        }
        if (!emacs.setBufferText(active()->path, active()->editBuf)) {
            outputLog += "[emacs] " + emacs.getLastError() + "\n";
            return;
        }
        outputLog += "[emacs] Pushed buffer to Emacs.\n";
    }

    void openInEmacsFrame() {
        if (!active()) return;
        if (active()->path.rfind("(untitled", 0) == 0) {
            outputLog += "[emacs] Save file before opening in Emacs.\n";
            return;
        }
        if (!emacs.openFileInEmacsFrame(active()->path)) {
            outputLog += "[emacs] " + emacs.getLastError() + "\n";
            return;
        }
        outputLog += "[emacs] Opened in Emacs frame.\n";
    }

    std::string buildRunCommand(const std::string& path,
                                const std::string& language,
                                bool buildOnly) const {
        std::filesystem::path p(path);
        std::string ext = p.extension().string();
        std::string quoted = "\"" + path + "\"";
        if (language == "python" || ext == ".py") {
            return buildOnly ? "python -m py_compile " + quoted : "python " + quoted;
        }
        if (language == "cpp" || ext == ".cpp" || ext == ".cc" || ext == ".cxx") {
            std::string out = (p.parent_path() / p.stem()).string();
#ifdef _WIN32
            out += ".exe";
#endif
            std::string compile = "g++ " + quoted + " -std=c++20 -o \"" + out + "\"";
            if (buildOnly) return compile;
            return compile + " && \"" + out + "\"";
        }
        if (language == "rust" || ext == ".rs") {
            return buildOnly ? "cargo build" : "cargo run";
        }
        if (language == "go" || ext == ".go") {
            return buildOnly ? "go build " + quoted : "go run " + quoted;
        }
        if (language == "javascript" || ext == ".js") {
            return buildOnly ? "node --check " + quoted : "node " + quoted;
        }
        if (language == "typescript" || ext == ".ts") {
            return buildOnly ? "tsc " + quoted : "node " + quoted;
        }
        return "";
    }

    bool runActiveFile(bool buildOnly) {
        if (!active()) return false;
        if (active()->path.rfind("(untitled", 0) == 0) {
            terminal.append("[terminal] Save the file before running.\n");
            showTerminalPanel = true;
            return false;
        }
        if (active()->modified) {
            doSave();
        }
        std::string cmd = buildRunCommand(active()->path, active()->language, buildOnly);
        if (cmd.empty()) {
            terminal.append("[terminal] No runner for language: " + active()->language + "\n");
            showTerminalPanel = true;
            return false;
        }
        std::string cwd = workspaceRoot.empty()
            ? std::filesystem::path(active()->path).parent_path().string()
            : workspaceRoot;
        showTerminalPanel = true;
        runInProgress = true;
        hasRunResult = false;
        lastRunCommand = cmd;
        int code = terminal.runAndAppend(cwd, cmd);
        terminal.append("[exit code: " + std::to_string(code) + "]\n");
        runInProgress = false;
        hasRunResult = true;
        lastRunExitCode = code;
        return code == 0;
    }

    // Called after editBuf changes (from ImGui input)
    void onTextChanged() {
        if (!active()) return;
        active()->editor.setContent(active()->editBuf, active()->language);
        if (active()->bufferMode == BufferManager::BufferMode::Structured) {
            active()->sync.setText(active()->editBuf, active()->language);
            active()->sync.syncNow();
            active()->incrementalOptimizer.setRoot(active()->sync.getAST());
        }
        primitives.setRoot(activeAST());
        primitives.setLanguage(active()->language);
        active()->orchestratorDirty = true;
        active()->highlightsDirty = true;
        active()->generatedHighlightsDirty = true;
        active()->modified = true;
        symbolsPending = true;
        symbolsLastChange = ImGui::GetTime();
        if (lsp) lsp->clearDocumentSymbols();
        if (lsp && active()->path.rfind("(untitled", 0) != 0) {
            active()->lspVersion += 1;
            lsp->didChange(toFileUri(active()->path), active()->editBuf, active()->lspVersion);
        }
        recordUndoSnapshot();
    }

    void doUndo() {
        if (!active()) return;
        std::string text;
        std::unique_ptr<Module> ast;
        if (active()->orchestrator.undoSnapshot(text, ast)) {
            applySnapshotToActive(text, std::move(ast));
        }
        active()->undoDepth = active()->orchestrator.getUndoDepth();
    }

    void doRedo() {
        if (!active()) return;
        std::string text;
        std::unique_ptr<Module> ast;
        if (active()->orchestrator.redoSnapshot(text, ast)) {
            applySnapshotToActive(text, std::move(ast));
        }
        active()->undoDepth = active()->orchestrator.getUndoDepth();
    }

    void doFind() {
        if (!active()) return;
        if (strlen(findBuf) == 0) return;
        int pos = active()->editor.find(findBuf, lastFindPos);
        if (pos >= 0) {
            lastFindPos = pos + 1;
            int line = 1, col = 1;
            for (int i = 0; i < pos && i < (int)active()->editBuf.size(); ++i) {
                if (active()->editBuf[i] == '\n') { ++line; col = 1; } else { ++col; }
            }
            active()->cursorLine = line;
            active()->cursorCol = col;
            outputLog += "Found \"" + std::string(findBuf) + "\" at line " +
                         std::to_string(line) + ", col " + std::to_string(col) + "\n";
        } else {
            lastFindPos = 0;
            outputLog += "\"" + std::string(findBuf) + "\" not found.\n";
        }
    }

    void doReplaceAll() {
        if (!active()) return;
        if (strlen(findBuf) == 0) return;
        int count = active()->editor.replaceAll(findBuf, replaceBuf);
        if (count > 0) {
            active()->editBuf = active()->editor.getContent();
            if (active()->bufferMode == BufferManager::BufferMode::Structured) {
                active()->sync.setText(active()->editBuf, active()->language);
                active()->sync.syncNow();
                active()->incrementalOptimizer.setRoot(active()->sync.getAST());
            }
            active()->highlightsDirty = true;
            active()->generatedHighlightsDirty = true;
            active()->modified = true;
        }
        outputLog += "Replaced " + std::to_string(count) + " occurrence(s).\n";
    }

    void doSave() {
        if (!active()) return;
        if (active()->path.rfind("(untitled", 0) == 0) return;
        std::ofstream out(active()->path);
        if (out.is_open()) {
            out << active()->editBuf;
            out.close();
            active()->modified = false;
            outputLog += "Saved: " + active()->path + "\n";
            if (lsp) lsp->didSave(toFileUri(active()->path), active()->editBuf);
        } else {
            outputLog += "Error saving: " + active()->path + "\n";
        }
    }

    void doOpen(const std::string& path,
                BufferManager::BufferMode modeOverride = BufferManager::BufferMode::Structured) {
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
            createBuffer(path, content, language, modeOverride);
            welcome.addRecentFile(path, language, bufferModeToString(modeOverride));
            saveRecentFiles();
            outputLog += "Opened: " + path + "\n";
        } else {
            outputLog += "Error opening: " + path + "\n";
        }
    }

    void reloadBuffer(const std::string& path) {
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
        buf->generatedHighlightsDirty = true;
        buf->modified = false;
        outputLog += "Reloaded: " + path + "\n";
    }

    void handleFileChanges() {
        auto changed = watcher.poll();
        for (const auto& path : changed) {
            auto it = bufferStates.find(path);
            if (it == bufferStates.end()) continue;
            if (it->second->modified) {
                outputLog += "File changed on disk (dirty): " + path + "\n";
            } else {
                reloadBuffer(path);
            }
        }
    }

    void updateHighlights() {
        if (!active()) return;
        if (!active()->highlightsDirty) return;
        active()->highlights = SyntaxHighlighter::highlight(active()->editBuf, active()->language);
        active()->highlightsDirty = false;
    }

    void updateGenerated() {
        if (!active()) return;
        if (active()->bufferMode == BufferManager::BufferMode::Text) return;
        Module* ast = active()->sync.getAST();
        std::string generated = generateForLanguage(ast, active()->generatedLanguage);
        if (generated != active()->generatedBuf) {
            active()->generatedBuf = generated;
            active()->generatedHighlightsDirty = true;
        }
        if (active()->generatedHighlightsDirty) {
            active()->generatedHighlights =
                SyntaxHighlighter::highlight(active()->generatedBuf, active()->generatedLanguage);
            active()->generatedHighlightsDirty = false;
        }
    }

    void projectToLanguage(const std::string& targetLanguage) {
        if (!active()) return;
        Module* ast = activeAST();
        if (!ast) {
            outputLog += "Project to " + targetLanguage + ": no AST available.\n";
            return;
        }

        CrossLanguageProjector projector;
        auto projected = projector.project(ast, targetLanguage);
        if (!projected) {
            outputLog += "Project to " + targetLanguage + ": failed to project AST.\n";
            return;
        }

        const int srcAnnoCount = countAnnotationNodes(ast);
        const int projAnnoCount = countAnnotationNodes(projected.get());
        const bool preserved = projector.annotationsPreserved(ast, projected.get());
        std::string generated = generateForLanguage(projected.get(), targetLanguage);

        std::string baseName = "(untitled-projection:" + targetLanguage + ")";
        std::string projName = baseName;
        int suffix = 1;
        while (buffers.hasBuffer(projName)) {
            projName = baseName + "-" + std::to_string(suffix++);
        }

        createBuffer(projName, generated, targetLanguage);
        if (active()) {
            active()->readOnly = true;
            active()->sync.setText(generated, targetLanguage);
            active()->sync.setAST(std::move(projected));
            active()->incrementalOptimizer.setRoot(active()->sync.getAST());
            active()->editBuf = generated;
            active()->editor.setContent(active()->editBuf, targetLanguage);
            active()->mode.setLanguage(targetLanguage);
            active()->highlightsDirty = true;
            active()->generatedLanguage = targetLanguage;
            active()->generatedMode.setLanguage(targetLanguage);
            active()->generatedHighlightsDirty = true;
            active()->modified = false;
        }

        outputLog += "Projected to " + targetLanguage + " in " + projName +
                     " (annotations " + std::to_string(srcAnnoCount) + " -> " +
                     std::to_string(projAnnoCount) + ", types preserved: " +
                     (preserved ? "yes" : "no") + ").\n";
    }

    void refreshActiveTextFromAST() {
        if (!active()) return;
        if (!isStructured()) return;
        active()->editBuf = active()->sync.getText();
        active()->editor.setContent(active()->editBuf, active()->language);
        active()->highlightsDirty = true;
        active()->modified = true;
        if (lsp && active()->path.rfind("(untitled", 0) != 0) {
            active()->lspVersion += 1;
            lsp->didChange(toFileUri(active()->path), active()->editBuf, active()->lspVersion);
        }
        analysisPending = true;
        analysisLastChange = ImGui::GetTime();
    }

    void openDiff(const std::string& beforeText,
                  const std::string& afterText,
                  bool preview,
                  int action,
                  const std::vector<std::string>& transformIds,
                  const std::vector<BatchMutationAPI::Mutation>& batchMutations = {}) {
        diff.active = true;
        diff.preview = preview;
        diff.action = action;
        diff.batch = !batchMutations.empty();
        diff.beforeText = beforeText;
        diff.afterText = afterText;
        diff.transformIds = transformIds;
        diff.batchMutations = batchMutations;
        LineDiff lineDiff = buildLineDiff(beforeText, afterText);
        diff.beforeLines = std::move(lineDiff.beforeLines);
        diff.afterLines = std::move(lineDiff.afterLines);
    }

    void updateCursorPos(int bytePos) {
        if (!active()) return;
        active()->cursorLine = 1;
        active()->cursorCol = 1;
        for (int i = 0; i < bytePos && i < (int)active()->editBuf.size(); ++i) {
            if (active()->editBuf[i] == '\n') { ++active()->cursorLine; active()->cursorCol = 1; }
            else { ++active()->cursorCol; }
        }
    }

    void refreshFileTree() {
        if (!fileTreeDirty) return;
        fileTreeRoot = fileTree.build(workspaceRoot);
        fileTreeDirty = false;
    }
};

struct NullLSPTransport : public LSPTransport {
    void send(const std::string& msg) override { (void)msg; }
    bool receive(std::string& out) override { (void)out; return false; }
    bool isOpen() const override { return false; }
    void close() override {}
};
