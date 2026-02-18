    if (method == "listBuffers") {
        json buffers = json::array();
        for (const auto& [path, buf] : state.bufferStates) {
            bool isActive = (buf.get() == state.activeBuffer);
            buffers.push_back({
                {"path", buf->path},
                {"language", buf->language},
                {"modified", buf->modified},
                {"active", isActive}
            });
        }
        return headlessRpcResult(id, {
            {"buffers", buffers},
            {"count", (int)buffers.size()},
            {"activeBuffer", state.activeBuffer
                ? state.activeBuffer->path : ""}
        });
    }

    // --- setActiveBuffer ---
    if (method == "setActiveBuffer") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");
        if (path.empty())
            return headlessRpcError(id, -32602, "Missing path parameter");
        if (!state.setActiveBuffer(path))
            return headlessRpcError(id, -32002, "Buffer not found: " + path);
        return headlessRpcResult(id, {
            {"activeBuffer", path},
            {"language", state.activeBuffer->language}
        });
    }

    // --- indexWorkspace ---
    if (method == "indexWorkspace") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string root = params.value("root", state.workspaceRoot);
        if (root.empty())
            return headlessRpcError(id, -32602,
                "No workspace root (set via --workspace or root param)");
        state.project.scanWorkspace(root);
        state.workspaceRoot = root;
        const auto& idx = state.project.index;
        return headlessRpcResult(id, {
            {"root", root},
            {"fileCount", idx.fileCount()},
            {"dirCount", idx.dirCount()},
            {"totalEntries", (int)idx.files().size()}
        });
    }

    // --- setWorkspaceContext ---
    if (method == "setWorkspaceContext") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string workspace = params.value("workspace", "");
        std::string language = params.value("language", "");
        if (workspace.empty()) {
            return headlessRpcError(id, -32602, "Missing workspace");
        }

        state.workspaceRoot = workspace;
        if (!language.empty()) state.defaultLanguage = language;
        state.project.scanWorkspace(workspace);
        const auto& idx = state.project.index;
        return headlessRpcResult(id, {
            {"workspace", state.workspaceRoot},
            {"language", state.defaultLanguage},
            {"fileCount", idx.fileCount()},
            {"dirCount", idx.dirCount()}
        });
    }

    // --- getImportGraph ---
    if (method == "getImportGraph") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string filePath = params.value("file", "");
        const auto& graph = state.project.importGraph;
        if (!filePath.empty()) {
            // Imports for a specific file
            auto imports = graph.importsOf(filePath);
            json importArr = json::array();
            for (const auto& m : imports) importArr.push_back(m);
            auto importers = graph.importedBy(
                fs::path(filePath).stem().string());
            json importerArr = json::array();
            for (const auto& f : importers) importerArr.push_back(f);
            return headlessRpcResult(id, {
                {"file", filePath},
                {"imports", importArr},
                {"importedBy", importerArr}
            });
        }
        // Full import graph
        json edges = json::object();
        for (const auto& [file, imports] : graph.edges()) {
            json importArr = json::array();
            for (const auto& m : imports) importArr.push_back(m);
            edges[file] = importArr;
        }
        return headlessRpcResult(id, {
            {"edges", edges},
            {"fileCount", (int)graph.edges().size()}
        });
    }

    // --- searchProject ---
    if (method == "searchProject") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string name = params.value("name", "");
        std::string nodeId = params.value("nodeId", "");

        // If nodeId given, resolve name from the active buffer
        if (!name.empty()) {
            // use name as-is
        } else if (!nodeId.empty()) {
            // Search all buffers for a node with this ID
            for (const auto& [path, buf] : state.bufferStates) {
                Module* ast = buf->sync.getAST();
                if (!ast) continue;
                ASTNode* node = findNodeById(ast, nodeId);
                if (node) {
                    name = getNodeName(node);
                    break;
                }
            }
            if (name.empty())
                return headlessRpcError(id, -32002,
                    "Node not found: " + nodeId);
        } else {
            return headlessRpcError(id, -32602,
                "Missing name or nodeId parameter");
        }

        json results = json::array();
        for (const auto& [path, buf] : state.bufferStates) {
            Module* ast = buf->sync.getAST();
            if (!ast) continue;
            auto refs = collectSymbolReferences(ast, name, path);
            for (const auto& ref : refs) {
                results.push_back({
                    {"file", ref.file}, {"line", ref.line},
                    {"col", ref.col}, {"nodeId", ref.nodeId},
                    {"kind", ref.kind}, {"context", ref.context}
                });
            }
        }

        std::set<std::string> searchFiles;
        for (const auto& r : results)
            searchFiles.insert(r.value("file", ""));

        return headlessRpcResult(id, {
            {"name", name},
            {"references", results},
            {"count", (int)results.size()},
            {"fileCount", (int)searchFiles.size()}
        });
    }

    // --- renameSymbol ---
    if (method == "renameSymbol") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string oldName = params.value("oldName", "");
        std::string newName = params.value("newName", "");
        bool preview = params.value("preview", false);

        if (oldName.empty() || newName.empty())
            return headlessRpcError(id, -32602,
                "Missing oldName or newName parameter");


        // Collect changes across all open buffers
        std::vector<RenameChange> allChanges;
        for (const auto& [path, buf] : state.bufferStates) {
            Module* ast = buf->sync.getAST();
            if (!ast) continue;
            auto changes = buildRenameChanges(
                ast, oldName, newName, path);
            allChanges.insert(allChanges.end(),
                              changes.begin(), changes.end());
        }

        if (allChanges.empty())
            return headlessRpcError(id, -32002,
                "No references found for '" + oldName + "'");

        // Build preview JSON
        json changesJson = json::array();
        std::set<std::string> affectedFiles;
        for (const auto& c : allChanges) {
            changesJson.push_back({
                {"file", c.file}, {"nodeId", c.nodeId},
                {"property", c.property},
                {"oldValue", c.oldValue},
                {"newValue", c.newValue},
                {"kind", c.kind}
            });
            affectedFiles.insert(c.file);
        }

        if (preview) {
            return headlessRpcResult(id, {
                {"preview", true},
                {"changes", changesJson},
                {"changeCount", (int)allChanges.size()},
                {"fileCount", (int)affectedFiles.size()}
            });
        }

        // Apply changes: use setProperty on each node
        int applied = 0;
        std::vector<std::string> errors;
        for (const auto& c : allChanges) {
            auto it = state.bufferStates.find(c.file);
            if (it == state.bufferStates.end()) continue;
            Module* ast = it->second->sync.getAST();
            if (!ast) continue;
            ASTNode* node = findNodeById(ast, c.nodeId);
            if (!node) {
                errors.push_back("Node " + c.nodeId +
                                 " not found in " + c.file);
                continue;
            }
            // Apply the property change directly
            if (node->conceptType == "Function" &&
                c.property == "name") {
                static_cast<Function*>(node)->name = c.newValue;
                ++applied;
            } else if (node->conceptType == "FunctionCall" &&
                       c.property == "functionName") {
                static_cast<FunctionCall*>(node)->functionName =
                    c.newValue;
                ++applied;
            } else if (node->conceptType == "Variable" &&
                       c.property == "name") {
                static_cast<Variable*>(node)->name = c.newValue;
                ++applied;
            } else if (node->conceptType == "VariableReference" &&
                       c.property == "variableName") {
                static_cast<VariableReference*>(node)->variableName =
                    c.newValue;
                ++applied;
            } else if (node->conceptType == "Parameter" &&
                       c.property == "name") {
                static_cast<Parameter*>(node)->name = c.newValue;
                ++applied;
            }
        }

        // Mark affected buffers as modified, regenerate editBuf, record undo
        for (const auto& f : affectedFiles) {
            auto it = state.bufferStates.find(f);
            if (it != state.bufferStates.end()) {
                it->second->modified = true;
                // Regenerate code from the modified AST
                Module* bufAST = it->second->sync.getAST();
                if (bufAST) {
                    Pipeline pipeline;
                    it->second->editBuf = pipeline.generate(
                        bufAST, it->second->language);
                    // Record post-rename state for undo
                    it->second->undoStack.record(
                        it->second->editBuf, bufAST);
                }
            }
        }

        json result = {
            {"applied", applied},
            {"changes", changesJson},
            {"changeCount", (int)allChanges.size()},
            {"fileCount", (int)affectedFiles.size()}
        };
        if (!errors.empty()) {
            json errArr = json::array();
            for (const auto& e : errors) errArr.push_back(e);
            result["errors"] = errArr;
        }
        return headlessRpcResult(id, result);
    }

    // --- getProjectDiagnostics ---
    if (method == "getProjectDiagnostics") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        // Optional severity filter
        std::string sevFilter = params.value("severity", "");
        // Optional file path glob filter (simple suffix match)
        std::string fileGlob = params.value("fileGlob", "");

        // Collect module names from open buffers for cross-file checks
        std::set<std::string> openModuleNames;
        for (const auto& [path, buf] : state.bufferStates) {
            std::string stem = fs::path(path).stem().string();
            if (!stem.empty()) openModuleNames.insert(stem);
        }

        json filesDiags = json::object();
        int totalCount = 0;

        for (const auto& [path, buf] : state.bufferStates) {
            // Apply file glob filter (simple suffix/extension match)
            if (!fileGlob.empty()) {
                // Match *.ext or exact name
                if (fileGlob[0] == '*') {
                    std::string suffix = fileGlob.substr(1);
                    if (path.size() < suffix.size() ||
                        path.substr(path.size() - suffix.size()) != suffix)
                        continue;
                } else if (path.find(fileGlob) == std::string::npos) {
                    continue;
                }
            }

            Module* bufAST = buf->sync.getAST();
            std::vector<StructuredDiagnostic> diags;

            // Per-file diagnostics (annotation + strategy)
            if (bufAST) {
                auto fileDiags = collectAllDiagnostics(bufAST);
                diags.insert(diags.end(), fileDiags.begin(),
                             fileDiags.end());
                // Cross-file: undefined imports from AST
                auto crossDiags = collectCrossFileDiagnostics(
                    bufAST, path, openModuleNames);
                diags.insert(diags.end(), crossDiags.begin(),
                             crossDiags.end());
            }

            // Cross-file: undefined imports from source text
            if (!buf->editBuf.empty()) {
                auto srcCross = collectCrossFileDiagnosticsFromSource(
                    buf->editBuf, path, openModuleNames);
                // Deduplicate: only add source-based if no AST-based
                // cross-file diags exist for the same line
                std::set<int> astCrossLines;
                for (const auto& d : diags) {
                    if (d.source == "cross-file")
                        astCrossLines.insert(d.line);
                }
                for (auto& d : srcCross) {
                    if (astCrossLines.find(d.line) == astCrossLines.end())
                        diags.push_back(std::move(d));
                }
            }

            // Apply severity filter
            if (!sevFilter.empty()) {
                DiagnosticSeverity maxSev = severityFromStr(sevFilter);
                diags = filterBySeverity(diags, maxSev);
            }

            if (!diags.empty()) {
                json diagArr = diagnosticsToJson(diags);
                sortDiagnosticsByPriority(diagArr);
                filesDiags[path] = diagArr;
                totalCount += (int)diags.size();
            }
        }

        return headlessRpcResult(id, {
            {"files", filesDiags},
            {"fileCount", (int)filesDiags.size()},
            {"totalDiagnostics", totalCount}
        });
    }

    // --- undo ---
    if (method == "undo") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32000, "No active buffer");
        auto& stack = state.activeBuffer->undoStack;
        if (!stack.canUndo())
            return headlessRpcError(id, -32050, "Nothing to undo");
        const auto& snap = stack.undo();
        // Restore AST from JSON
        if (!snap.astJson.is_null()) {
            ASTNode* node = fromJson(snap.astJson);
            if (node && node->conceptType == "Module") {
                state.activeBuffer->sync.setAST(
                    std::unique_ptr<Module>(
                        static_cast<Module*>(node)));
                state.activeBuffer->orchestratorDirty = true;
            } else {
                deleteTree(node);
            }
        }
        state.activeBuffer->editBuf = snap.text;
        state.activeBuffer->modified = true;
        return headlessRpcResult(id, {
            {"success", true},
            {"undoDepth", stack.undoDepth()},
            {"redoDepth", stack.redoDepth()}
        });
    }

    // --- redo ---
    if (method == "redo") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32000, "No active buffer");
        auto& stack = state.activeBuffer->undoStack;
        if (!stack.canRedo())
            return headlessRpcError(id, -32050, "Nothing to redo");
        const auto& snap = stack.redo();
        if (!snap.astJson.is_null()) {
            ASTNode* node = fromJson(snap.astJson);
            if (node && node->conceptType == "Module") {
                state.activeBuffer->sync.setAST(
                    std::unique_ptr<Module>(
                        static_cast<Module*>(node)));
                state.activeBuffer->orchestratorDirty = true;
            } else {
                deleteTree(node);
            }
        }
        state.activeBuffer->editBuf = snap.text;
        state.activeBuffer->modified = true;
        return headlessRpcResult(id, {
            {"success", true},
            {"undoDepth", stack.undoDepth()},
            {"redoDepth", stack.redoDepth()}
        });
    }

    // --- saveBuffer ---
    if (method == "saveBuffer") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");

        // Default to active buffer if no path specified
        if (path.empty()) {
            if (!state.activeBuffer)
                return headlessRpcError(id, -32000, "No active buffer");
            path = state.activeBuffer->path;
        }

        auto it = state.bufferStates.find(path);
        if (it == state.bufferStates.end())
            return headlessRpcError(id, -32002,
                "Buffer not found: " + path);

        auto& buf = it->second;

        // Resolve path against workspace root
        std::string writePath = path;
        if (!state.workspaceRoot.empty()) {
            auto [ok, resolved] =
                fileOpsResolvePath(state.workspaceRoot, path);
            if (!ok)
                return headlessRpcError(id, -32040, resolved);
            writePath = resolved;
        }

        // Create parent directories if needed
        fs::path parentDir = fs::path(writePath).parent_path();
        if (!parentDir.empty()) {
            std::error_code ec;
            fs::create_directories(parentDir, ec);
        }

        // Write editBuf to disk
        auto [success, msg, bytes] = fileOpsWrite(writePath, buf->editBuf);
        if (!success)
            return headlessRpcError(id, -32041, msg);

        buf->modified = false;
        return headlessRpcResult(id, {
            {"success", true}, {"path", writePath},
            {"bytesWritten", bytes}
        });
    }

    // --- saveAllBuffers ---
