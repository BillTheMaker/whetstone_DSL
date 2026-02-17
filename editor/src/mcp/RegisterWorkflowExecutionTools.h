    void registerWorkflowExecutionTools() {
        // whetstone_create_workflow
        tools_.push_back({"whetstone_create_workflow",
            "Create a workflow from the active buffer's skeleton AST. "
            "Populates work items from skeleton tasks with routing annotations.",
            {{"type", "object"}, {"properties", {
                {"projectName", {{"type", "string"},
                    {"description", "Name for the workflow project"}}}
            }}, {"required", json::array({"projectName"})}}
        });
        toolHandlers_["whetstone_create_workflow"] =
            [this](const json& args) {
                return callWhetstone("createWorkflow", args);
            };

        // whetstone_get_workflow_state
        tools_.push_back({"whetstone_get_workflow_state",
            "Get current workflow state including phase, stats, ready and blocked counts.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_workflow_state"] =
            [this](const json& args) {
                return callWhetstone("getWorkflowState", args);
            };

        // whetstone_get_ready_tasks
        tools_.push_back({"whetstone_get_ready_tasks",
            "Get work items ready for assignment, ordered by priority.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_ready_tasks"] =
            [this](const json& args) {
                return callWhetstone("getReadyTasks", args);
            };

        // whetstone_get_work_item
        tools_.push_back({"whetstone_get_work_item",
            "Get full details of a single work item including result if present.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Work item ID"}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_get_work_item"] =
            [this](const json& args) {
                return callWhetstone("getWorkItem", args);
            };

        // whetstone_assign_task
        tools_.push_back({"whetstone_assign_task",
            "Assign a ready work item to a worker.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Work item ID to assign"}}},
                {"assignee", {{"type", "string"},
                    {"description", "Worker identifier"}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_assign_task"] =
            [this](const json& args) {
                return callWhetstone("assignTask", args);
            };

        // whetstone_complete_task
        tools_.push_back({"whetstone_complete_task",
            "Mark a work item as complete with generated result. "
            "Triggers dependency cascade — blocked items may become ready.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Work item ID to complete"}}},
                {"result", {{"type", "object"}, {"properties", {
                    {"generatedCode", {{"type", "string"},
                        {"description", "Generated code output"}}},
                    {"confidence", {{"type", "number"},
                        {"description", "Confidence score 0.0-1.0"}}},
                    {"reasoning", {{"type", "string"},
                        {"description", "Explanation of decisions"}}}
                }}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_complete_task"] =
            [this](const json& args) {
                return callWhetstone("completeTask", args);
            };

        // whetstone_reject_task
        tools_.push_back({"whetstone_reject_task",
            "Reject a work item back to the queue with feedback.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Work item ID to reject"}}},
                {"reason", {{"type", "string"},
                    {"description", "Rejection reason/feedback"}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_reject_task"] =
            [this](const json& args) {
                return callWhetstone("rejectTask", args);
            };

        // whetstone_save_workflow
        tools_.push_back({"whetstone_save_workflow",
            "Persist the current workflow state to a sidecar JSON file.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_save_workflow"] =
            [this](const json& args) {
                return callWhetstone("saveWorkflow", args);
            };
    }

