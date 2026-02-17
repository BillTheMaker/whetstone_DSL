    void registerRoutingTools() {
        // whetstone_route_task
        tools_.push_back({"whetstone_route_task",
            "Route a single work item — determine worker type, context width, "
            "budget, and review requirements based on annotations.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Work item ID to route"}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_route_task"] =
            [this](const json& args) {
                return callWhetstone("routeTask", args);
            };

        // whetstone_route_all_ready
        tools_.push_back({"whetstone_route_all_ready",
            "Route all ready work items in the workflow, applying routing "
            "decisions based on their annotations.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_route_all_ready"] =
            [this](const json& args) {
                return callWhetstone("routeAllReady", args);
            };

        // whetstone_execute_task
        tools_.push_back({"whetstone_execute_task",
            "Execute a work item using its assigned worker. Deterministic and "
            "template workers produce code directly; agent/human workers prepare "
            "context bundles for external invocation.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Work item ID to execute"}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_execute_task"] =
            [this](const json& args) {
                return callWhetstone("executeTask", args);
            };

        // whetstone_get_routing_explanation
        tools_.push_back({"whetstone_get_routing_explanation",
            "Explain why a work item was routed to a specific worker type. "
            "Returns reasoning, annotations used, and rules applied.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Work item ID to explain"}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_get_routing_explanation"] =
            [this](const json& args) {
                return callWhetstone("getRoutingExplanation", args);
            };
    }

