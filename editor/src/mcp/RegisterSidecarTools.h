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

        // whetstone_save_semantic_hash_table
        tools_.push_back({"whetstone_save_semantic_hash_table",
            "Compute and persist semantic hash table sidecar for the current "
            "buffer. Uses SemAnno/Whetstone semantic primitives as hash inputs.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Buffer path to save semantic hash table for"}}}
            }}}
        });
        toolHandlers_["whetstone_save_semantic_hash_table"] =
            [this](const json& args) {
                return callWhetstone("saveSemanticHashTable", args);
            };

        // whetstone_get_semantic_hash_table
        tools_.push_back({"whetstone_get_semantic_hash_table",
            "Read semantic hash table sidecar for a file. Set refresh=true "
            "to recompute from live AST before reading.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Buffer path to read semantic hash table for"}}},
                {"refresh", {{"type", "boolean"},
                    {"description", "Recompute hash table from AST before read"}}}
            }}}
        });
        toolHandlers_["whetstone_get_semantic_hash_table"] =
            [this](const json& args) {
                return callWhetstone("getSemanticHashTable", args);
            };

        // whetstone_list_semantic_hash_tables
        tools_.push_back({"whetstone_list_semantic_hash_tables",
            "List all semantic hash table sidecar files in the workspace.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_list_semantic_hash_tables"] =
            [this](const json& args) {
                return callWhetstone("listSemanticHashTables", args);
            };

        // whetstone_set_semantic_hash_lock
        tools_.push_back({"whetstone_set_semantic_hash_lock",
            "Set hash lock state on an AST node (locked/unlocked) with "
            "optional reason. This is scaffolding metadata for lock policy.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description", "AST node ID"}}},
                {"locked", {{"type", "boolean"},
                    {"description", "true=locked, false=unlocked"}}},
                {"reason", {{"type", "string"},
                    {"description", "Optional lock rationale"}}}
            }}, {"required", {"nodeId", "locked"}}}
        });
        toolHandlers_["whetstone_set_semantic_hash_lock"] =
            [this](const json& args) {
                return callWhetstone("setSemanticHashLock", args);
            };

        // whetstone_get_semantic_hash_lock
        tools_.push_back({"whetstone_get_semantic_hash_lock",
            "Get semantic hash lock metadata for an AST node.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description", "AST node ID"}}}
            }}, {"required", {"nodeId"}}}
        });
        toolHandlers_["whetstone_get_semantic_hash_lock"] =
            [this](const json& args) {
                return callWhetstone("getSemanticHashLock", args);
            };
    }

    // ---------------------------------------------------------------
    //  Step 269: Semantic annotation management tools
    // ---------------------------------------------------------------
