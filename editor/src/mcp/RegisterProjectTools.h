    void registerProjectTools() {
        // whetstone_open_file
        tools_.push_back({"whetstone_open_file",
            "Open a file as a buffer for AST analysis. Reads from disk "
            "if content not provided. Language auto-detected from extension.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "File path (relative to workspace or absolute)"}}},
                {"content", {{"type", "string"},
                    {"description", "File content (optional, reads from disk if omitted)"}}},
                {"language", {{"type", "string"},
                    {"description", "Language (optional, auto-detected from extension)"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_open_file"] = [this](const json& args) {
            return callWhetstone("openFile", args);
        };

        // whetstone_close_file
        tools_.push_back({"whetstone_close_file",
            "Close an open buffer, removing it from the project.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Path of the buffer to close"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_close_file"] = [this](const json& args) {
            return callWhetstone("closeFile", args);
        };

        // whetstone_list_buffers
        tools_.push_back({"whetstone_list_buffers",
            "List all open buffers with language, modified status, and "
            "which buffer is currently active.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_list_buffers"] = [this](const json& args) {
            return callWhetstone("listBuffers", args);
        };

        // whetstone_set_active_buffer
        tools_.push_back({"whetstone_set_active_buffer",
            "Switch the active buffer. Subsequent getAST, getDiagnostics, "
            "etc. will operate on this buffer.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Path of the buffer to activate"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_set_active_buffer"] = [this](const json& args) {
            return callWhetstone("setActiveBuffer", args);
        };

        // whetstone_index_workspace
        tools_.push_back({"whetstone_index_workspace",
            "Scan the workspace directory and index all file paths. "
            "Returns file and directory counts. Does not open files.",
            {{"type", "object"}, {"properties", {
                {"root", {{"type", "string"},
                    {"description", "Workspace root (optional, uses --workspace default)"}}}
            }}}
        });
        toolHandlers_["whetstone_index_workspace"] = [this](const json& args) {
            return callWhetstone("indexWorkspace", args);
        };

        // whetstone_search_project
        tools_.push_back({"whetstone_search_project",
            "Find all references to a symbol across all open files. "
            "Search by name or nodeId. Returns file, line, col, nodeId, "
            "kind (definition/call/reference/parameter), and context.",
            {{"type", "object"}, {"properties", {
                {"name", {{"type", "string"},
                    {"description",
                     "Symbol name to search for"}}},
                {"nodeId", {{"type", "string"},
                    {"description",
                     "Node ID to resolve name from (alternative to name)"}}}
            }}}
        });
        toolHandlers_["whetstone_search_project"] =
            [this](const json& args) {
                return callWhetstone("searchProject", args);
            };

        // whetstone_rename_symbol
        tools_.push_back({"whetstone_rename_symbol",
            "Rename a symbol across all open files. Updates function "
            "definitions, calls, variable declarations, references, and "
            "parameters. Set preview=true to see changes without applying.",
            {{"type", "object"}, {"properties", {
                {"oldName", {{"type", "string"},
                    {"description", "Current symbol name"}}},
                {"newName", {{"type", "string"},
                    {"description", "New symbol name"}}},
                {"preview", {{"type", "boolean"},
                    {"description",
                     "Preview changes without applying (default false)"}}}
            }}, {"required", {"oldName", "newName"}}}
        });
        toolHandlers_["whetstone_rename_symbol"] =
            [this](const json& args) {
                return callWhetstone("renameSymbol", args);
            };
    }

    // ---------------------------------------------------------------
    //  Save and undo/redo tools
    // ---------------------------------------------------------------
