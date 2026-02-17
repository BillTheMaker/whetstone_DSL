    void registerDiagnosticTools() {
        tools_.push_back({"whetstone_get_diagnostics",
            "Get structured diagnostics for the active buffer. Combines "
            "parse errors, annotation validation, and strategy violations "
            "into one stream with error codes, nodeIds, and fix suggestions. "
            "Filter by severity (error/warning/info/hint) or source "
            "(parser/annotation/strategy).",
            {{"type", "object"}, {"properties", {
                {"severity", {{"type", "string"},
                    {"enum", {"error", "warning", "info", "hint"}},
                    {"description", "Maximum severity level to include"}}},
                {"source", {{"type", "string"},
                    {"enum", {"parser", "annotation", "strategy"}},
                    {"description", "Filter by diagnostic source"}}}
            }}}
        });
        toolHandlers_["whetstone_get_diagnostics"] = [this](const json& args) {
            return callWhetstone("getDiagnostics", args);
        };

        // whetstone_get_diagnostics_delta
        tools_.push_back({"whetstone_get_diagnostics_delta",
            "Get only the diagnostics that changed since a given version. "
            "Returns added and removed diagnostics for efficient "
            "mutate-then-check loops. Use the version from a previous "
            "getDiagnostics or getDiagnosticsDelta response.",
            {{"type", "object"}, {"properties", {
                {"sinceVersion", {{"type", "integer"},
                    {"description",
                     "Version number from a previous diagnostics response"}}}
            }}, {"required", {"sinceVersion"}}}
        });
        toolHandlers_["whetstone_get_diagnostics_delta"] =
            [this](const json& args) {
                return callWhetstone("getDiagnosticsDelta", args);
            };

        // whetstone_get_quick_fixes
        tools_.push_back({"whetstone_get_quick_fixes",
            "Get all applicable quick-fix actions for a node or the entire "
            "AST. Each fix is a concrete mutation object the agent can review "
            "and apply with whetstone_apply_quick_fix.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description",
                     "Node ID to get fixes for (omit for all fixes)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_quick_fixes"] = [this](const json& args) {
            return callWhetstone("getQuickFixes", args);
        };

        // whetstone_apply_quick_fix
        tools_.push_back({"whetstone_apply_quick_fix",
            "Apply a quick-fix action for a specific diagnostic. Takes the "
            "diagnostic code and nodeId, finds the fix, and applies it as a "
            "mutation. Reports whether the diagnostic was cleared.",
            {{"type", "object"}, {"properties", {
                {"diagCode", {{"type", "string"},
                    {"description", "Diagnostic error code (e.g. E0200)"}}},
                {"nodeId", {{"type", "string"},
                    {"description", "Target node ID"}}}
            }}, {"required", {"diagCode", "nodeId"}}}
        });
        toolHandlers_["whetstone_apply_quick_fix"] = [this](const json& args) {
            return callWhetstone("applyQuickFix", args);
        };

        // whetstone_get_project_diagnostics
        tools_.push_back({"whetstone_get_project_diagnostics",
            "Get diagnostics across all open files in one call. Returns "
            "diagnostics grouped by file path in compact format. Includes "
            "cross-file errors (undefined imports). Filter by severity "
            "or file glob pattern.",
            {{"type", "object"}, {"properties", {
                {"severity", {{"type", "string"},
                    {"enum", {"error", "warning", "info", "hint"}},
                    {"description",
                     "Maximum severity level to include"}}},
                {"fileGlob", {{"type", "string"},
                    {"description",
                     "File pattern filter (e.g. *.py, utils.py)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_project_diagnostics"] =
            [this](const json& args) {
                return callWhetstone("getProjectDiagnostics", args);
            };
    }

    // ---------------------------------------------------------------
    //  Register all tools
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    //  Batch query tool
    // ---------------------------------------------------------------
