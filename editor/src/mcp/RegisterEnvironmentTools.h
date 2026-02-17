    void registerEnvironmentTools() {
        // whetstone_set_environment
        tools_.push_back({"whetstone_set_environment",
            "Set the environment spec on the active module. Defines "
            "scheduler, memory model, capabilities, constraints, "
            "exception model, and FFI style. Replaces any existing env.",
            {{"type", "object"}, {"properties", {
                {"envId", {{"type", "string"},
                    {"description", "Environment identifier (e.g. posix_process, browser, jvm)"}}},
                {"scheduler", {{"type", "string"},
                    {"enum", {"event_loop", "threads", "fibers", "coroutines", "single_thread"}},
                    {"description", "Scheduling model"}}},
                {"memory", {{"type", "string"},
                    {"enum", {"manual", "raii", "refcount", "tracing_gc", "region"}},
                    {"description", "Memory management model"}}},
                {"capabilities", {{"type", "array"}, {"items", {{"type", "string"}}},
                    {"description", "Available capabilities (io.fs, io.net, threads, etc.)"}}},
                {"constraints", {{"type", "array"}, {"items", {{"type", "string"}}},
                    {"description", "Environment constraints (no_jit, no_threads, etc.)"}}},
                {"exceptions", {{"type", "string"},
                    {"enum", {"unwind", "checked", "typed", "untyped"}},
                    {"description", "Exception handling model"}}},
                {"ffi", {{"type", "string"},
                    {"enum", {"c_abi", "dynamic_linking", "none"}},
                    {"description", "Foreign function interface style"}}}
            }}, {"required", {"envId"}}}
        });
        toolHandlers_["whetstone_set_environment"] =
            [this](const json& args) {
                return callWhetstone("setEnvironment", args);
            };

        // whetstone_get_environment
        tools_.push_back({"whetstone_get_environment",
            "Get the current environment spec from the active module. "
            "Returns hasEnvironment=false if none is set.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_environment"] =
            [this](const json& args) {
                return callWhetstone("getEnvironment", args);
            };

        // whetstone_validate_environment
        tools_.push_back({"whetstone_validate_environment",
            "Validate the active module against its environment spec. "
            "Checks capability requirements (E0501) and annotation "
            "compatibility (E0502-E0505). Returns diagnostics array.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_validate_environment"] =
            [this](const json& args) {
                return callWhetstone("validateEnvironment", args);
            };

        // whetstone_get_lowering_hints
        tools_.push_back({"whetstone_get_lowering_hints",
            "Get environment-aware lowering hints for a node. Returns "
            "code generation patterns based on scheduler, memory, and "
            "exception models (e.g. callback, std_async, try_catch).",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description",
                     "Node ID (optional, defaults to module root)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_lowering_hints"] =
            [this](const json& args) {
                return callWhetstone("getLoweringHints", args);
            };
    }

