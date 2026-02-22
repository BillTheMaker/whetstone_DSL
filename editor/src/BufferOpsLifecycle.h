#pragma once
// --- BufferOpsLifecycle.h ---
// Extracted from BufferOps.h (Sprint 8 cleanup).
// Session metadata and lifecycle initialization helpers.
// Included from BufferOps.h after the EditorState struct definition.

inline std::string EditorState::osLabel() {
#ifdef _WIN32
    return "windows";
#elif __APPLE__
    return "macos";
#else
    return "linux";
#endif
}

inline std::string EditorState::detectProjectType() const {
    std::filesystem::path root(workspaceRoot);
    std::error_code ec;
    auto exists = [&](const std::string& name) {
        return std::filesystem::exists(root / name, ec);
    };
    if (exists("CMakeLists.txt")) return "cmake";
    if (exists("package.json")) return "node";
    if (exists("pyproject.toml") || exists("requirements.txt")) return "python";
    if (exists("Cargo.toml")) return "rust";
    if (exists("go.mod")) return "go";
    if (exists("pom.xml") || exists("build.gradle")) return "java";
    return "unknown";
}

inline json EditorState::buildSessionMetadata(bool autoRecording) const {
    json meta;
    meta["editorVersion"] = welcome.getVersion();
    meta["os"] = osLabel();
    meta["language"] = activeBuffer ? activeBuffer->language : std::string("unknown");
    meta["projectType"] = detectProjectType();
    meta["autoRecording"] = autoRecording;
    return meta;
}

inline void EditorState::startSessionRecording(const std::string& name, bool autoRecording) {
    WorkflowRecorder::RecordingConfig cfg;
    cfg.recordAllSessions = true;
    cfg.recordControlMethods = true;
    cfg.recordEditorEvents = true;
    cfg.autoRecording = autoRecording;
    agent.workflowRecorder.startRecording(name, "", cfg, buildSessionMetadata(autoRecording));
    agent.workflowRecorder.recordEvent("session_start", {{"name", name}, {"autoRecording", autoRecording}});
    notify(NotificationLevel::Info, "Session recording started.");
}

inline void EditorState::stopSessionRecording() {
    if (!agent.workflowRecorder.isRecording()) return;
    agent.workflowRecorder.recordEvent("session_stop", json::object());
    agent.workflowRecorder.stopRecording();
    notify(NotificationLevel::Info, "Session recording stopped.");
}

inline void EditorState::toggleSessionRecording() {
    if (agent.workflowRecorder.isRecording()) stopSessionRecording();
    else startSessionRecording("session", false);
}

inline void EditorState::recordUIEvent(const UIEvent& ev) {
    json payload = {
        {"path", ev.path},
        {"detail", ev.detail},
        {"uiTimestamp", ev.timestamp}
    };
    agent.workflowRecorder.recordEvent(uiEventLabel(ev.type), payload);
}

inline void EditorState::recordEditorEvent(const std::string& type, const json& payload) {
    agent.workflowRecorder.recordEvent(type, payload);
}

inline void EditorState::init() {
    notify(NotificationLevel::Info, "Whetstone Editor ready.");
    workspaceRoot.clear();
    search.projectSearch.setRoot("");
    fileTreeRoot = FileNode{};
    fileTreeDirty = true;
    loadRecentFiles();
    loadFeatureHints(featureHints, workspaceRoot);
    loadSettingsFromDisk();
    if (settings.getAutoRecordSessions() && !agent.workflowRecorder.isRecording()) {
        startSessionRecording("auto-session", true);
    }
    registerCommands();
    library.vulnDb.startBackgroundRefresh();
    library.semanticTags.load();
    events.subscribe(UIEventType::BufferSwitched, [this](const UIEvent& ev) {
        if (!active()) return;
        library.primitives.setRoot(activeAST());
        library.primitives.setLanguage(active()->language);
        symbolsPending = true;
        symbolsLastChange = ImGui::GetTime();
        if (lsp) lsp->clearDocumentSymbols();
        if (active()->language == "elisp") {
            emacsState.emacsFunctionIndexDirty = true;
        }
        recordUIEvent(ev);
    });
    events.subscribe(UIEventType::ASTChanged, [this](const UIEvent&) {
        if (!active()) return;
        library.primitives.setRoot(activeAST());
        library.primitives.setLanguage(active()->language);
        symbolsPending = true;
        symbolsLastChange = ImGui::GetTime();
        if (lsp) lsp->clearDocumentSymbols();
    });
    events.subscribe(UIEventType::SettingsChanged, [this](const UIEvent& ev) {
        applySettingsToState();
        recordUIEvent(ev);
    });
    events.subscribe(UIEventType::ThemeChanged, [this](const UIEvent& ev) {
        recordUIEvent(ev);
    });
    events.subscribe(UIEventType::FileModified, [this](const UIEvent& ev) {
        recordUIEvent(ev);
    });
    events.subscribe(UIEventType::NotificationPosted, [this](const UIEvent& ev) {
        recordUIEvent(ev);
    });
    startEmacsDaemonFromSettings();
    initAgentServer();
    refreshBuildSystem();
    telemetry.initialize(workspaceRoot, settings.getTelemetryOptIn());
    telemetry.installCrashHandler();
}
