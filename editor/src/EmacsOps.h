#pragma once
// --- EmacsOps.h (Step 239) ---
// Extracted from EditorState.h (Sprint 8).
// Included from EditorState.h after the EditorState struct definition.

inline std::string EditorState::orgTempExtension(const std::string& language) const {
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

inline std::string EditorState::writeOrgTempFile(const std::string& language, const std::string& code) {
    std::filesystem::path root = workspaceRoot.empty()
        ? std::filesystem::current_path()
        : std::filesystem::path(workspaceRoot);
    std::filesystem::path dir = root / ".whetstone_org";
    std::filesystem::create_directories(dir);
    std::string name = "org_block_" + std::to_string(++emacsState.orgTempCounter) +
                       orgTempExtension(language);
    std::filesystem::path filePath = dir / name;
    std::ofstream out(filePath.string(), std::ios::binary);
    out << code;
    return filePath.string();
}

inline std::string EditorState::runOrgBlock(const std::string& language, const std::string& code) {
    if (language == "elisp") {
        std::string result = emacsState.emacs.sendCommand(ElispCommandBuilder::eval(code));
        if (!emacsState.emacs.getLastError().empty() && result == "error") {
            return emacsState.emacs.getLastError();
        }
        return result.empty() ? "OK" : result;
    }
    std::string path = writeOrgTempFile(language, code);
    std::string cmd = buildRunCommand(path, language, false);
    if (cmd.empty()) return "No runner for language: " + language;
    build.showTerminalPanel = true;
    std::string output;
    int codeExit = build.terminal.runAndCapture(workspaceRoot, cmd, output);
    output += "[exit code: " + std::to_string(codeExit) + "]";
    return output;
}

inline void EditorState::startEmacsDaemonFromSettings() {
    emacsDiagnostics.clear();
    std::string initPath = resolveEmacsInitPath();
    std::string log;
    bool ok = emacsState.emacs.startDaemonWithConfig(initPath, log);
    if (!log.empty()) {
        notify(NotificationLevel::Info, "[emacs] " + log);
    }
    if (!ok) {
        EditorDiagnostic d;
        d.uri = "emacs://init";
        d.severity = 1;
        d.message = "[Emacs] " + emacsState.emacs.getLastError();
        d.source = "EmacsDaemon";
        emacsDiagnostics.push_back(d);
        return;
    }
    if (!log.empty()) {
        addEmacsLogDiagnostics(log);
    }
}

inline std::string EditorState::resolveEmacsInitPath() const {
    return ::resolveEmacsInitPath(settings.getEmacsConfigPath());
}

inline void EditorState::addEmacsLogDiagnostics(const std::string& log) {
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

inline void EditorState::initAgentServer() {
    auto transport = std::make_unique<MockWebSocketTransport>();
    agent.transport = transport.get();
    agent.server = std::make_unique<WebSocketAgentServer>(std::move(transport));
    agent.server->setRequestHandler([this](const json& request,
                                           const std::string& sessionId) {
        return processAgentRequest(request, sessionId);
    });
    agent.server->setSessionEventCallback([this](const AgentSession& s,
                                                const std::string& event) {
        logAgentEvent("Agent " + s.sessionId + " " + event);
        if (event == "connected") {
            agent.roles[s.sessionId] = agent.defaultRole;
            recordEditorEvent("agent_connected", {{"sessionId", s.sessionId}, {"agentName", s.agentName}});
        } else if (event == "disconnected") {
            agent.roles.erase(s.sessionId);
            recordEditorEvent("agent_disconnected", {{"sessionId", s.sessionId}});
        }
    });
    agent.server->setRequestLogCallback([this](const std::string& sid,
                                               const json& req,
                                               const json& res) {
        std::string method = req.value("method", "");
        bool ok = !res.contains("error");
        logAgentEvent("RPC " + sid + " " + method + (ok ? " ok" : " error"));
        agent.workflowRecorder.record(sid, req, res);
    });
    if (agent.server->start(agent.port)) {
        logAgentEvent("Agent server started on port " + std::to_string(agent.port));
    } else {
        logAgentEvent("Agent server failed to start on port " + std::to_string(agent.port));
    }
}

inline void EditorState::shutdownAgentServer() {
    if (agent.server) agent.server->stop();
}

inline void EditorState::logAgentEvent(const std::string& msg) {
    agent.log.push_back(msg);
}

inline void EditorState::updateEmacsFunctionIndex() {
    if (!active() || active()->language != "elisp") return;
    std::string error;
    auto packages = queryEmacsPackageList(emacsState.emacs, false, notifications, error);
    if (!error.empty()) return;
    refreshEmacsFunctionIndex(emacsState.emacsFunctionIndex, emacsState.emacs, packages, notifications);
    addEmacsSymbolsToLibraryIndex(library.libraryIndex, emacsState.emacsFunctionIndex);
    emacsState.emacsFunctionIndexDirty = false;
    rebuildExternalModulesFromIndex();
    notify(NotificationLevel::Info,
           "[emacs] Indexed functions for " +
           std::to_string(emacsState.emacsFunctionIndex.functionsByPackage.size()) +
           " packages.");
}

inline bool EditorState::handleEmacsKeyChord(const std::string& chord) {
    if (ui.layoutPreset != LayoutPreset::Emacs) return false;
    return emacsHandleKeySequence(emacsState.emacsKeys, emacsState.emacs, chord, notifications);
}

inline void EditorState::refreshEmacsModeLine(double nowSeconds) {
    if (ui.layoutPreset != LayoutPreset::Emacs) return;
    updateEmacsModeLine(emacsState.emacsKeys, emacsState.emacs, nowSeconds, notifications);
}

inline void EditorState::pullFromEmacs() {
    if (!active()) return;
    if (active()->path.rfind("(untitled", 0) == 0) {
        notify(NotificationLevel::Warning, "[emacs] Save file before syncing.");
        return;
    }
    std::string text = emacsState.emacs.getBufferText(active()->path);
    if (!emacsState.emacs.getLastError().empty() && text == "error") {
        notify(NotificationLevel::Error,
               "[emacs] " + emacsState.emacs.getLastError());
        return;
    }
    active()->editBuf = text;
    onTextChanged();
    notify(NotificationLevel::Success, "[emacs] Pulled buffer from Emacs.");
}

inline void EditorState::pushToEmacs() {
    if (!active()) return;
    if (active()->path.rfind("(untitled", 0) == 0) {
        notify(NotificationLevel::Warning, "[emacs] Save file before syncing.");
        return;
    }
    if (!emacsState.emacs.setBufferText(active()->path, active()->editBuf)) {
        notify(NotificationLevel::Error,
               "[emacs] " + emacsState.emacs.getLastError());
        return;
    }
    notify(NotificationLevel::Success, "[emacs] Pushed buffer to Emacs.");
}

inline void EditorState::openInEmacsFrame() {
    if (!active()) return;
    if (active()->path.rfind("(untitled", 0) == 0) {
        notify(NotificationLevel::Warning,
               "[emacs] Save file before opening in Emacs.");
        return;
    }
    if (!emacsState.emacs.openFileInEmacsFrame(active()->path)) {
        notify(NotificationLevel::Error,
               "[emacs] " + emacsState.emacs.getLastError());
        return;
    }
    notify(NotificationLevel::Success, "[emacs] Opened in Emacs frame.");
}

inline std::string EditorState::buildRunCommand(const std::string& path,
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

inline bool EditorState::runActiveFile(bool buildOnly) {
    if (!active()) return false;
    if (active()->path.rfind("(untitled", 0) == 0) {
        build.terminal.append("[terminal] Save the file before running.\n");
        build.showTerminalPanel = true;
        return false;
    }
    if (active()->modified) {
        doSave();
    }
    std::string cmd = buildRunCommand(active()->path, active()->language, buildOnly);
    if (cmd.empty()) {
        build.terminal.append("[terminal] No runner for language: " + active()->language + "\n");
        build.showTerminalPanel = true;
        return false;
    }
    std::string cwd = workspaceRoot.empty()
        ? std::filesystem::path(active()->path).parent_path().string()
        : workspaceRoot;
    build.showTerminalPanel = true;
    build.runInProgress = true;
    build.hasRunResult = false;
    build.lastRunCommand = cmd;
    int code = build.terminal.runAndAppend(cwd, cmd);
    build.terminal.append("[exit code: " + std::to_string(code) + "]\n");
    build.runInProgress = false;
    build.hasRunResult = true;
    build.lastRunExitCode = code;
    return code == 0;
}

