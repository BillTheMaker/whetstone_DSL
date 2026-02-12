#pragma once
// --- LspOps.h (Step 238) ---
// Extracted from EditorState.h (Sprint 8).
// Included from EditorState.h after the EditorState struct definition.

inline void EditorState::ensureImportForSymbol(const std::string& libraryName, const std::string& symbol) {
    if (!active() || libraryName.empty()) return;
    if (settings.getBlockVulnerableImports()) {
        PackageEcosystem eco = ecosystemForLanguage(active()->language);
        std::string osvEco = osvEcosystem(eco);
        if (!osvEco.empty()) {
            std::string key = vulnKey(osvEco, libraryName);
            if (library.dependencyPanel.vulnIgnore.find(key) ==
                library.dependencyPanel.vulnIgnore.end()) {
                std::string version = dependencyVersionFor(osvEco, libraryName);
                auto vulns = library.vulnDb.query(osvEco, libraryName, version);
                if (!vulns.empty()) {
                    notify(NotificationLevel::Warning,
                           "Blocked vulnerable import: " + libraryName);
                    return;
                }
            }
        }
    }
    std::string clean = symbol;
    auto paren = clean.find('(');
    if (paren != std::string::npos) clean = clean.substr(0, paren);
    ImportEditResult result = ensureImport(active()->editBuf,
                                           active()->language,
                                           libraryName,
                                           clean);
    if (result.changed) {
        active()->editBuf = result.text;
        onTextChanged();
    }
}

inline void EditorState::appendUnusedImportDiagnostics(std::vector<EditorDiagnostic>& diags) {
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

inline std::string EditorState::dependencyVersionFor(const std::string& osvEco,
    const std::string& package) const {
    if (package.empty()) return "";
    for (const auto& dep : library.dependencyPanel.deps) {
        if (dep.name != package) continue;
        PackageEcosystem eco = ecosystemForSource(dep.source);
        if (osvEcosystem(eco) != osvEco) continue;
        return dep.version;
    }
    return "";
}

inline void EditorState::appendVulnerabilityDiagnostics(std::vector<EditorDiagnostic>& diags) {
    if (!active()) return;
    PackageEcosystem eco = ecosystemForLanguage(active()->language);
    std::string osvEco = osvEcosystem(eco);
    if (osvEco.empty()) return;
    auto imports = collectImportLocations(active()->editBuf, active()->language);
    if (imports.empty()) return;
    std::string uri = toFileUri(active()->path);
    for (const auto& imp : imports) {
        std::string key = vulnKey(osvEco, imp.library);
        if (library.dependencyPanel.vulnIgnore.find(key) !=
            library.dependencyPanel.vulnIgnore.end()) {
            continue;
        }
        std::string version = dependencyVersionFor(osvEco, imp.library);
        auto vulns = library.vulnDb.query(osvEco, imp.library, version);
        if (vulns.empty()) continue;
        const auto& top = vulns.front();
        int severity = 3;
        if (top.severity == "Critical" || top.severity == "High") severity = 1;
        else if (top.severity == "Medium") severity = 2;
        EditorDiagnostic ed;
        ed.uri = uri;
        ed.line = imp.line;
        ed.character = 0;
        ed.severity = severity;
        ed.message = "[Security] " + imp.library + ": " + top.summary;
        if (!top.cveId.empty()) ed.message += " (" + top.cveId + ")";
        ed.source = "VulnerabilityDB";
        diags.push_back(std::move(ed));
    }
}

inline void EditorState::flushLspDidChange(double nowSeconds) {
    if (!lsp || !lspChangePending) return;
    double delay = settings.getLspDebounceMs() / 1000.0;
    if ((nowSeconds - lspChangeLast) < delay) return;
    auto it = bufferStates.find(lspChangePath);
    if (it == bufferStates.end()) {
        lspChangePending = false;
        return;
    }
    auto* buf = it->second.get();
    lsp->didChange(toFileUri(buf->path), buf->editBuf, buf->lspVersion);
    lspChangePending = false;
}

inline void EditorState::flushDeferredAstSync(double nowSeconds) {
    (void)nowSeconds;
    for (auto& kv : bufferStates) {
        auto* buf = kv.second.get();
        if (!buf->pendingAstSync) continue;
        buf->sync.setText(buf->editBuf, buf->language);
        buf->sync.syncNow();
        buf->incrementalOptimizer.setRoot(buf->sync.getAST());
        buf->pendingAstSync = false;
        buf->highlightsDirty = true;
        buf->generatedHighlightsDirty = true;
        break;
    }
}

inline bool EditorState::hasSidePanels() const {
    if (ui.showOutline) return true;
    if (library.showDependencyPanel) return true;
    if (library.showLibraryBrowserPanel) return true;
    if (library.showCompositionPanel) return true;
    if (emacsState.showEmacsPackagesPanel) return true;
    if (emacsState.showEmacsBridgePanel) return true;
    return true; // Memory Strategies panel is always present.
}

inline void EditorState::cyclePanelFocus() {
    FocusRegion order[] = {
        FocusRegion::Editor,
        FocusRegion::Explorer,
        FocusRegion::Side,
        FocusRegion::Bottom
    };
    int current = 0;
    for (int i = 0; i < 4; ++i) {
        if (ui.focusedRegion == order[i]) {
            current = i;
            break;
        }
    }
    for (int step = 1; step <= 4; ++step) {
        FocusRegion next = order[(current + step) % 4];
        if (next == FocusRegion::Side && !hasSidePanels()) continue;
        ui.focusTarget = next;
        return;
    }
}

inline void EditorState::refreshBuildSystem() {
    build.buildType = BuildSystem::detect(workspaceRoot);
}

inline int EditorState::runBuildCommand(const BuildCommand& cmd) {
    if (workspaceRoot.empty()) {
        build.terminal.append("[build] No workspace root set.\n");
        return -1;
    }
    build.showTerminalPanel = true;
    build.lastBuildCommand = cmd.command;
    int code = build.terminal.runAndCapture(workspaceRoot, cmd.command, build.lastBuildOutput);
    build.buildErrors = BuildSystem::parseErrors(build.lastBuildOutput);
    notify(NotificationLevel::Info,
           "[build] " + cmd.label + " => exit " + std::to_string(code));
    return code;
}

inline void EditorState::pollLspMessages() {
    if (!lsp || !lspTransport || !lspTransport->isOpen()) return;
    std::string msg;
    while (lspTransport->receive(msg)) {
        lsp->handleMessage(msg);
    }
}

inline void EditorState::requestLibraryIndex() {
    library.libraryIndex.symbolsByLibrary.clear();
    library.libraryIndex.completionsByLibrary.clear();
    library.libraryIndexRequests.clear();
    if (!isStructured() || !activeAST()) {
        notify(NotificationLevel::Warning,
               "[deps] Library index skipped: no structured AST.");
        return;
    }
    if (!lsp || !lspTransport || !lspTransport->isOpen()) {
        applyStubFallback(library.libraryIndex, library.dependencyPanel.deps, workspaceRoot);
        rebuildExternalModulesFromIndex();
        notify(NotificationLevel::Warning,
               "[deps] Library index uses cached symbols (LSP offline).");
        return;
    }

    for (const auto& dep : library.dependencyPanel.deps) {
        if (dep.name.empty()) continue;
        LibraryState::LibraryIndexRequest req;
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
        library.libraryIndexRequests.push_back(req);
    }
    applyStubFallback(library.libraryIndex, library.dependencyPanel.deps, workspaceRoot);
    rebuildExternalModulesFromIndex();
    notify(NotificationLevel::Info,
           "[deps] Library index requests queued: " +
           std::to_string(library.libraryIndexRequests.size()));
}

inline void EditorState::processLibraryIndexResponses() {
    if (!lsp) return;
    bool updated = false;
    for (auto& req : library.libraryIndexRequests) {
        if (req.symbolsRequestId >= 0) {
            std::vector<LSPClient::WorkspaceSymbol> symbols;
            if (lsp->takeWorkspaceSymbols(req.symbolsRequestId, symbols)) {
                library.libraryIndex.symbolsByLibrary[req.name] = std::move(symbols);
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
                    library.libraryIndex.completionsByLibrary[req.name] = std::move(labels);
                }
                req.completionRequestId = -1;
                updated = true;
            }
        }
    }
    if (updated) {
        applyStubFallback(library.libraryIndex, library.dependencyPanel.deps, workspaceRoot);
        rebuildExternalModulesFromIndex();
        notify(NotificationLevel::Info, "[deps] Library index updated.");
    }
}

inline void EditorState::rebuildExternalModulesFromIndex() {
    Module* ast = activeAST();
    if (!ast) return;
    rebuildExternalModules(ast,
                           library.dependencyPanel.deps,
                           library.libraryIndex,
                           &library.semanticTags);
    if (active() && active()->language == "elisp") {
        int signatureId = 0;
        appendEmacsExternalModules(ast, emacsState.emacsFunctionIndex, signatureId);
    }
    if (active()) active()->orchestratorDirty = true;
}

inline json EditorState::buildDiagnosticsJson() const {
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

inline PackageEcosystem EditorState::ecosystemForLanguage(const std::string& language) {
    if (language == "python") return PackageEcosystem::Python;
    if (language == "javascript" || language == "typescript") return PackageEcosystem::Npm;
    if (language == "rust") return PackageEcosystem::Rust;
    if (language == "go") return PackageEcosystem::Go;
    if (language == "java") return PackageEcosystem::Java;
    return PackageEcosystem::Cpp;
}

inline std::vector<EditorState::ImportLocation> EditorState::collectImportLocations(const std::string& text,
    const std::string& language) {
    std::vector<EditorState::ImportLocation> out;
    auto lines = importSplitLines(text);
    for (int i = 0; i < (int)lines.size(); ++i) {
        std::string t = trimStr(lines[i]);
        if (language == "python") {
            if (startsWith(t, "import ")) {
                std::string lib = trimStr(t.substr(7));
                auto comma = lib.find(',');
                if (comma != std::string::npos) lib = trimStr(lib.substr(0, comma));
                out.push_back({i, lib});
            } else if (startsWith(t, "from ")) {
                auto pos = t.find("import");
                if (pos != std::string::npos) {
                    std::string lib = trimStr(t.substr(5, pos - 5));
                    out.push_back({i, lib});
                }
            }
        } else if (language == "javascript" || language == "typescript") {
            if (startsWith(t, "import ")) {
                auto pos = t.find(" from ");
                if (pos != std::string::npos) {
                    auto quote = t.find('\'', pos);
                    if (quote == std::string::npos) quote = t.find('\"', pos);
                    if (quote != std::string::npos) {
                        auto end = t.find(t[quote], quote + 1);
                        if (end != std::string::npos) {
                            std::string lib = t.substr(quote + 1, end - quote - 1);
                            out.push_back({i, lib});
                        }
                    }
                }
            }
        } else if (language == "rust") {
            if (startsWith(t, "use ")) {
                std::string lib = trimStr(t.substr(4));
                auto pos = lib.find("::");
                if (pos != std::string::npos) lib = lib.substr(0, pos);
                if (!lib.empty() && lib.back() == ';') lib.pop_back();
                out.push_back({i, lib});
            }
        } else if (language == "go") {
            if (startsWith(t, "import \"")) {
                std::string path = t.substr(8);
                if (!path.empty() && path.back() == '"') path.pop_back();
                out.push_back({i, path});
            }
        } else if (language == "elisp") {
            if (startsWith(t, "(require '")) {
                auto start = t.find('\'');
                auto end = t.find(')', start);
                if (start != std::string::npos && end != std::string::npos) {
                    std::string lib = t.substr(start + 1, end - start - 1);
                    out.push_back({i, lib});
                }
            }
        } else if (language == "cpp") {
            if (startsWith(t, "#include")) {
                auto lt = t.find('<');
                auto gt = t.find('>');
                if (lt != std::string::npos && gt != std::string::npos && gt > lt) {
                    out.push_back({i, t.substr(lt + 1, gt - lt - 1)});
                } else {
                    auto q = t.find('"');
                    if (q != std::string::npos) {
                        auto q2 = t.find('"', q + 1);
                        if (q2 != std::string::npos) {
                            out.push_back({i, t.substr(q + 1, q2 - q - 1)});
                        }
                    }
                }
            }
        }
    }
    return out;
}

inline AgentRole EditorState::getAgentRole(const std::string& sessionId) const {
    auto it = agent.roles.find(sessionId);
    return it != agent.roles.end() ? it->second : AgentRole::Linter;
}

inline void EditorState::setAgentRole(const std::string& sessionId, AgentRole role) {
    agent.roles[sessionId] = role;
}

inline std::string EditorState::agentActorLabel(const std::string& sessionId) const {
    std::string label = "agent:" + sessionId;
    if (agent.server) {
        if (const auto* sess = agent.server->getSession(sessionId)) {
            if (!sess->agentName.empty()) {
                label = "agent:" + sess->agentName;
            }
        }
    }
    return label;
}
