    void registerFileTools() {
        // whetstone_file_read
        tools_.push_back({"whetstone_file_read",
            "Read a file from the workspace. Returns file content with "
            "optional line range filtering. Path is relative to workspace root.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}, {"description", "File path (relative to workspace)"}}},
                {"startLine", {{"type", "integer"}, {"description", "Start line (1-based, optional)"}}},
                {"endLine", {{"type", "integer"}, {"description", "End line (1-based, optional)"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_file_read"] = [this](const json& args) {
            return callWhetstone("fileRead", args);
        };

        // whetstone_file_write
        tools_.push_back({"whetstone_file_write",
            "Write content to a file in the workspace. Creates parent "
            "directories if needed. Requires Refactor or Generator role.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}, {"description", "File path (relative to workspace)"}}},
                {"content", {{"type", "string"}, {"description", "Content to write"}}}
            }}, {"required", {"path", "content"}}}
        });
        toolHandlers_["whetstone_file_write"] = [this](const json& args) {
            return callWhetstone("fileWrite", args);
        };

        // whetstone_file_create
        tools_.push_back({"whetstone_file_create",
            "Create a new file with optional language template. "
            "Requires Refactor or Generator role.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}, {"description", "File path (relative to workspace)"}}},
                {"language", {{"type", "string"}, {"description", "Language (python, cpp, rust, go, java, javascript, typescript)"}}},
                {"template", {{"type", "string"}, {"description", "Template name (module, empty) or empty for blank file"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_file_create"] = [this](const json& args) {
            return callWhetstone("fileCreate", args);
        };

        // whetstone_file_diff
        tools_.push_back({"whetstone_file_diff",
            "Get a unified diff between the active buffer content and the "
            "file on disk. Optionally specify a path, defaults to active buffer.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}, {"description", "File path (optional, defaults to active buffer)"}}}
            }}}
        });
        toolHandlers_["whetstone_file_diff"] = [this](const json& args) {
            return callWhetstone("fileDiff", args);
        };

        // whetstone_workspace_list
        tools_.push_back({"whetstone_workspace_list",
            "List files in the workspace matching an optional glob pattern. "
            "Respects .gitignore. Returns path, size, and isDir for each entry.",
            {{"type", "object"}, {"properties", {
                {"glob", {{"type", "string"}, {"description", "Glob pattern (default: *). E.g. *.py, *.cpp"}}}
            }}}
        });
        toolHandlers_["whetstone_workspace_list"] = [this](const json& args) {
            return callWhetstone("workspaceList", args);
        };
    }

    // ---------------------------------------------------------------
    //  Step 250: Register diagnostic tools
    // ---------------------------------------------------------------
