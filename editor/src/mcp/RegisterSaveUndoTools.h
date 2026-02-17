    void registerSaveUndoTools() {
        // whetstone_save_buffer
        tools_.push_back({"whetstone_save_buffer",
            "Save a buffer to disk. Writes the current editBuf (code "
            "regenerated from AST) to the file path. Clears the "
            "modified flag. Defaults to active buffer if no path given.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description",
                     "Buffer path to save (optional, defaults to active)"}}}
            }}}
        });
        toolHandlers_["whetstone_save_buffer"] =
            [this](const json& args) {
                return callWhetstone("saveBuffer", args);
            };

        // whetstone_save_all_buffers
        tools_.push_back({"whetstone_save_all_buffers",
            "Save all modified buffers to disk. Skips unmodified "
            "buffers. Returns count of saved and skipped files.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_save_all_buffers"] =
            [this](const json& args) {
                return callWhetstone("saveAllBuffers", args);
            };

        // whetstone_undo
        tools_.push_back({"whetstone_undo",
            "Undo the last mutation on the active buffer. Restores "
            "the previous AST and regenerated code from the snapshot "
            "journal. Returns the new undo/redo depths.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_undo"] =
            [this](const json& args) {
                return callWhetstone("undo", args);
            };

        // whetstone_redo
        tools_.push_back({"whetstone_redo",
            "Redo the last undone mutation on the active buffer. "
            "Returns the new undo/redo depths.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_redo"] =
            [this](const json& args) {
                return callWhetstone("redo", args);
            };
    }

    // ---------------------------------------------------------------
    //  Step 267: Sidecar persistence tools
    // ---------------------------------------------------------------
