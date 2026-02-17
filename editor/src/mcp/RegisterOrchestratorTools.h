    void registerOrchestratorTools() {
        // whetstone_orchestrate_step
        tools_.push_back({"whetstone_orchestrate_step",
            "Advance one ready workflow item through the orchestration loop. "
            "Returns one orchestrator event (routed/executed/completed/blocked).",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_orchestrate_step"] =
            [this](const json& args) {
                return callWhetstone("orchestrateStep", args);
            };

        // whetstone_orchestrate_advance
        tools_.push_back({"whetstone_orchestrate_advance",
            "Advance all currently ready workflow items in one batch. "
            "Returns event stream and batch counters.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_orchestrate_advance"] =
            [this](const json& args) {
                return callWhetstone("orchestrateAdvance", args);
            };

        // whetstone_orchestrate_run_deterministic
        tools_.push_back({"whetstone_orchestrate_run_deterministic",
            "Run deterministic/template workflow items to completion. "
            "Stops automatically at human or external-model blockers.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_orchestrate_run_deterministic"] =
            [this](const json& args) {
                return callWhetstone("orchestrateRunDeterministic", args);
            };

        // whetstone_get_blockers
        tools_.push_back({"whetstone_get_blockers",
            "Get current workflow blockers grouped by type: needs-human, "
            "needs-review, and needs-external-model.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_blockers"] =
            [this](const json& args) {
                return callWhetstone("getBlockers", args);
            };

        // whetstone_get_progress
        tools_.push_back({"whetstone_get_progress",
            "Get workflow progress snapshot: completion %, throughput, ETA, "
            "worker stats, and active blockers.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_progress"] =
            [this](const json& args) {
                return callWhetstone("getProgress", args);
            };

        // whetstone_submit_result
        tools_.push_back({"whetstone_submit_result",
            "Submit external SLM/LLM output for a prepared workflow item. "
            "Orchestrator applies review gate and advances lifecycle.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Workflow item ID receiving external output"}}},
                {"result", {{"type", "object"}, {"properties", {
                    {"generatedCode", {{"type", "string"}}},
                    {"confidence", {{"type", "number"}}},
                    {"reasoning", {{"type", "string"}}},
                    {"tokensGenerated", {{"type", "integer"}}},
                    {"tokensBudget", {{"type", "integer"}}},
                    {"astJson", {{"type", "object"}}},
                    {"diagnostics", {{"type", "array"}, {"items", {{"type", "object"}}}}}
                }}}}
            }}, {"required", json::array({"itemId", "result"})}}
        });
        toolHandlers_["whetstone_submit_result"] =
            [this](const json& args) {
                return callWhetstone("submitExternalResult", args);
            };

        // whetstone_get_event_stream
        tools_.push_back({"whetstone_get_event_stream",
            "Poll workflow events emitted since a stream version. "
            "Returns normalized event records for visualization and clients.",
            {{"type", "object"}, {"properties", {
                {"sinceVersion", {{"type", "integer"},
                    {"description", "Return events with version > sinceVersion"}}}
            }}}
        });
        toolHandlers_["whetstone_get_event_stream"] =
            [this](const json& args) {
                return callWhetstone("getEventStream", args);
            };

        // whetstone_get_recent_events
        tools_.push_back({"whetstone_get_recent_events",
            "Get the latest N workflow events from the event stream.",
            {{"type", "object"}, {"properties", {
                {"count", {{"type", "integer"},
                    {"description", "How many most-recent events to return (default 20)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_recent_events"] =
            [this](const json& args) {
                return callWhetstone("getRecentEvents", args);
            };
    }

