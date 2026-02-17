    void registerSidecarTools() {
        // whetstone_save_annotated_ast
        tools_.push_back({"whetstone_save_annotated_ast",
            "Save the current buffer's AST (with all semantic annotations) "
            "to a .whetstone/ sidecar file. Annotations persist alongside "
            "the codebase without polluting source code.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Buffer path to save annotations for"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_save_annotated_ast"] =
            [this](const json& args) {
                return callWhetstone("saveAnnotatedAST", args);
            };

        // whetstone_load_annotated_ast
        tools_.push_back({"whetstone_load_annotated_ast",
            "Load semantic annotations from a .whetstone/ sidecar file "
            "and merge them into the current buffer's AST. Matches "
            "annotations to live nodes by node ID.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Buffer path to load annotations for"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_load_annotated_ast"] =
            [this](const json& args) {
                return callWhetstone("loadAnnotatedAST", args);
            };

        // whetstone_list_annotated_files
        tools_.push_back({"whetstone_list_annotated_files",
            "List all files that have saved .whetstone/ sidecar annotation "
            "files in the workspace.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_list_annotated_files"] =
            [this](const json& args) {
                return callWhetstone("listAnnotatedFiles", args);
            };
    }

    // ---------------------------------------------------------------
    //  Step 269: Semantic annotation management tools
    // ---------------------------------------------------------------
