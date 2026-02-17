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
#include "BatchMutationAPI.h"
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
#include "AgentRegistry.h"
#include "AgentMarketplace.h"
#include "BuildSystem.h"
#include "HelpPanel.h"
#include "Telemetry.h"
#include "UpdateChecker.h"
#include "NotificationSystem.h"
#include "UIEventBus.h"
#include "SearchUtils.h"
#include "FeatureHints.h"
#include "state/SearchState.h"
#include "state/AgentState.h"
#include "state/BuildState.h"
#include "state/LibraryState.h"
#include "state/EmacsState.h"
#include "state/UIFlags.h"
#include "state/UIAnimationState.h"
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
    if (mode == "structured") return BufferManager::BufferMode::Structured;
    return BufferManager::BufferMode::Text;
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
    BufferManager::BufferMode bufferMode = BufferManager::BufferMode::Text;
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
    size_t            fileSizeBytes = 0;
    bool              largeFileMode = false;
    bool              disableSyntaxHighlight = false;
    double            highlightRequestTime = 0.0;
    bool              pendingAstSync = false;
    double            pendingAstStart = 0.0;
};

struct EditorState {
    // Rendering context (set by main before first frame)
    ImFont*           monoFont = nullptr;
    ImFont*           uiFont = nullptr;
    float             baseFontSize = 15.0f;
    bool              fontsDirty = false;
    std::string       lastDialogPath;
    bool              exitRequested = false;

    KeybindingManager keys;
    BufferManager     buffers;
    std::map<std::string, std::unique_ptr<BufferState>> bufferStates;
    BufferState*      activeBuffer = nullptr;

    SearchState       search;
    AgentState        agent;
    BuildState        build;
    LibraryState      library;
    EmacsState        emacsState;
    UIFlags           ui;
    UIAnimationState  uiAnimations;
    bool              showLargeFilePrompt = false;
    std::string       largeFilePromptPath;
    size_t            largeFilePromptBytes = 0;

    NotificationSystem notifications;
    UIEventBus         events;
    HelpPanelState helpPanel;
    Telemetry telemetry;
    FeatureHintsState featureHints;
    char              outlineFilter[128] = {};
    WelcomeScreen     welcome;
    std::string       workspaceRoot;
    FileTree          fileTree;
    FileNode          fileTreeRoot;
    bool              fileTreeDirty = true;
    FileWatcher       watcher;
    std::shared_ptr<LSPTransport> lspTransport;
    std::shared_ptr<LSPClient> lsp;
    bool              symbolsPending = false;
    double            symbolsLastChange = 0.0;
    bool              completionPending = false;
    bool              completionVisible = false;
    bool              completionDismissed = false;
    int               completionSelected = 0;
    double            completionLastChange = 0.0;
    bool              lspChangePending = false;
    double            lspChangeLast = 0.0;
    std::string       lspChangePath;
    bool              hoverPending = false;
    double            hoverLastMove = 0.0;
    ImVec2            hoverLastMouse = ImVec2(-1.0f, -1.0f);
    bool              definitionPending = false;
    double            definitionLastRequest = 0.0;
    int               definitionRequestLine = 0;
    int               definitionRequestCol = 0;
    std::string       definitionRequestPath;
    double            lastFrameBudgetWarning = 0.0;
    LSPClient::DefinitionLocation definitionPreview;
    bool              definitionPreviewValid = false;
    Pipeline          pipeline;
    bool              analysisPending = false;
    double            analysisLastChange = 0.0;
    std::vector<EditorDiagnostic> whetstoneDiagnostics;
    std::vector<EditorDiagnostic> emacsDiagnostics;
    SettingsManager   settings;
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
    std::unordered_map<std::string, std::function<void(const std::string&)>> commandHandlers;
    bool              showCommandPalette = false;
    char              commandQuery[128] = {};
    int               commandSelected = 0;
    char              commandInlineValue[64] = {};
    std::string       commandInlineCommandId;
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

    static std::string fromFileUri(const std::string& uri);

    static int byteOffsetForLineCol(const std::string& text, int lineZero, int colZero);

    static int wordStartAt(const std::string& text, int pos);

    static bool isWordChar(char c) {
        return std::isalnum((unsigned char)c) || c == '_';
    }

    static std::string wordAt(const std::string& text, int pos);

    static std::string wordAtLineCol(const std::string& text, int lineZero, int colZero);

    static std::string wordPrefixAt(const std::string& text, int pos);

    void notify(NotificationLevel level, const std::string& message) {
        notifications.notify(level, message);
        events.publish(UIEventType::NotificationPosted, {}, message, ImGui::GetTime());
    }

    void notify(NotificationLevel level, const std::string& message, const NotificationTarget& target) {
        notifications.notify(level, message, target);
        events.publish(UIEventType::NotificationPosted, target.path, message, ImGui::GetTime());
    }

    void navigateToTarget(const NotificationTarget& target);

    void jumpTo(BufferState* buf, int lineZero, int colZero) {
        if (!buf) return;
        int pos = byteOffsetForLineCol(buf->editBuf, lineZero, colZero);
        buf->widget.setCursor(pos);
        buf->cursorLine = lineZero + 1;
        buf->cursorCol = colZero + 1;
    }

    void applyCompletion(BufferState* buf, const std::string& insertText, int replaceStart, int replaceEnd);

    void insertTextAtCursor(const std::string& text);

    void ensureImportForSymbol(const std::string& library, const std::string& symbol);

    void appendUnusedImportDiagnostics(std::vector<EditorDiagnostic>& diags);

    struct ImportLocation {
        int line = 0;
        std::string library;
    };

    static PackageEcosystem ecosystemForLanguage(const std::string& language);

    static std::vector<ImportLocation> collectImportLocations(const std::string& text,
                                                              const std::string& language);

    std::string dependencyVersionFor(const std::string& osvEco,
                                     const std::string& package) const;

    void appendVulnerabilityDiagnostics(std::vector<EditorDiagnostic>& diags);

    std::string makeUntitledName() const;

    void createBuffer(const std::string& path, const std::string& content,
                      const std::string& language,
                      BufferManager::BufferMode mode = BufferManager::BufferMode::Text,
                      size_t fileSizeBytes = 0,
                      bool largeFileMode = false,
                      bool disableSyntaxHighlight = false,
                      bool deferAstSync = false);
    BufferManager::BufferMode defaultBufferMode() const;

    bool jumpToDefinitionLocation(const LSPClient::DefinitionLocation& loc);

    bool goToDefinitionFallback(int lineZero, int colZero);

    bool goToDefinitionAt(int lineZero, int colZero);

    bool previewDefinitionAt(int lineZero, int colZero, std::string& outLabel);

    void switchToBuffer(const std::string& path);

    static std::string osLabel();

    std::string detectProjectType() const;

    json buildSessionMetadata(bool autoRecording) const;

    void startSessionRecording(const std::string& name, bool autoRecording);

    void stopSessionRecording();

    void toggleSessionRecording();

    static std::string uiEventLabel(UIEventType type);

    void recordUIEvent(const UIEvent& ev);

    void recordEditorEvent(const std::string& type, const json& payload);

    std::filesystem::path configDir() const;

    std::filesystem::path recentFilePath() const;

    void loadRecentFiles();

    void saveRecentFiles();

    void closeAllBuffers();

    bool renameBufferPath(const std::string& oldPath, const std::string& newPath);

    bool saveProject(const std::string& path);

    bool loadProject(const std::string& path);

    std::filesystem::path sessionFilePath() const;

    std::filesystem::path settingsFilePath() const;

    bool saveSession(const std::string& imguiIni);

    bool loadSession(SessionData& out);

    void applySession(const SessionData& session);

    void applyTabSizeToBuffers(int size);

    void setActiveBufferMode(BufferManager::BufferMode mode);

    void queueLspDidChange();

    void flushLspDidChange(double nowSeconds);

    void flushDeferredAstSync(double nowSeconds);

    bool hasSidePanels() const;

    void cyclePanelFocus();

    void applySettingsToState();

    void loadSettingsFromDisk();

    void saveSettingsToDisk();

    void init();

    std::string orgTempExtension(const std::string& language) const;

    std::string writeOrgTempFile(const std::string& language, const std::string& code);

    std::string runOrgBlock(const std::string& language, const std::string& code);

    void startEmacsDaemonFromSettings();

    std::string resolveEmacsInitPath() const;

    void addEmacsLogDiagnostics(const std::string& log);

    void initAgentServer();

    void shutdownAgentServer();

    void logAgentEvent(const std::string& msg);

    AgentRole getAgentRole(const std::string& sessionId) const;

    void setAgentRole(const std::string& sessionId, AgentRole role);

    std::string agentActorLabel(const std::string& sessionId) const;

    void refreshBuildSystem();

    int runBuildCommand(const BuildCommand& cmd);

    void pollLspMessages();

    void requestLibraryIndex();

    void processLibraryIndexResponses();

    void updateEmacsFunctionIndex();

    void rebuildExternalModulesFromIndex();

    json buildDiagnosticsJson() const;

    json processAgentRequest(const json& request, const std::string& sessionId);


    void syncOrchestratorFromActive();

    void applyOrchestratorToActive();

    Module* mutationAST();

    void recordUndoSnapshot();

    void applySnapshotToActive(const std::string& text, std::unique_ptr<Module> ast);

    void registerCommand(const std::string& id,
                         const std::string& label,
                         const std::string& shortcut,
                         const std::string& category,
                         int contextMask,
                         std::function<void()> fn);

    void registerCommand(const std::string& id,
                         const std::string& label,
                         const std::string& shortcut,
                         const std::string& category,
                         int contextMask,
                         const std::string& inputHint,
                         std::function<void(const std::string&)> fn);

    void registerCommands();

    void executeCommand(const std::string& id, const std::string& arg = {});

    void setLanguage(const std::string& lang);

    bool handleEmacsKeyChord(const std::string& chord);

    void refreshEmacsModeLine(double nowSeconds);

    void pullFromEmacs();

    void pushToEmacs();

    void openInEmacsFrame();

    std::string buildRunCommand(const std::string& path,
                                const std::string& language,
                                bool buildOnly) const;

    bool runActiveFile(bool buildOnly);

    // Called after editBuf changes (from ImGui input)
    void onTextChanged();

    void doUndo();

    void doRedo();

    SearchOptions currentSearchOptions() const;

    void addSearchHistory(const std::string& query);

    bool selectionRange(int& outStart, int& outEnd);

    void refreshSearchMatches();

    bool moveToMatchIndex(int index);

    void doFindNext(bool backwards = false);

    void doReplaceCurrent();

    void doFind();

    void doReplaceAll();

    void doSave();

    void doOpen(const std::string& path,
                BufferManager::BufferMode modeOverride = BufferManager::BufferMode::Text,
                bool deferAstSync = false);

    void reloadBuffer(const std::string& path);

    void handleFileChanges();

    void updateHighlights();

    void updateGenerated();

    void projectToLanguage(const std::string& targetLanguage);

    void refreshActiveTextFromAST();

    void openDiff(const std::string& beforeText,
                  const std::string& afterText,
                  bool preview,
                  int action,
                  const std::vector<std::string>& transformIds,
                  const std::vector<BatchMutationAPI::Mutation>& batchMutations = {});

    void updateCursorPos(int bytePos);

    void refreshFileTree();
};

// --- LspOps (extracted Step 238) ---
#include "LspOps.h"

// --- EmacsOps (extracted Step 239) ---
#include "EmacsOps.h"

// --- EditOps (extracted Step 237) ---
#include "EditOps.h"

// --- BufferOps (extracted Step 236) ---
#include "BufferOps.h"

// --- AgentRPCHandler (extracted Step 235) ---
#include "AgentRPCHandler.h"

inline json EditorState::processAgentRequest(const json& request,
                                             const std::string& sessionId) {
    return handleAgentRequest(*this, request, sessionId);
}

#include "state/NullLSPTransport.h"
