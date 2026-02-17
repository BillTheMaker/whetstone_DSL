    void registerWorkflowTools() {
        // whetstone_create_skeleton
        tools_.push_back({"whetstone_create_skeleton",
            "Create a new skeleton module — an architect's project specification "
            "with annotated function/class signatures but no implementation. "
            "Returns bufferId for the new skeleton module.",
            {{"type", "object"}, {"properties", {
                {"name", {{"type", "string"},
                    {"description", "Module name"}}},
                {"language", {{"type", "string"},
                    {"description", "Target language (python, cpp, rust, etc.)"}}}
            }}, {"required", json::array({"name", "language"})}}
        });
        toolHandlers_["whetstone_create_skeleton"] =
            [this](const json& args) {
                return callWhetstone("createSkeleton", args);
            };

        // whetstone_add_skeleton_node
        tools_.push_back({"whetstone_add_skeleton_node",
            "Add a function or class skeleton with routing annotations to the "
            "active skeleton module. Annotations control how the task is dispatched: "
            "contextWidth (local/file/project), automatability (deterministic/template/slm/llm/human), "
            "priority (critical/high/medium/low).",
            {{"type", "object"}, {"properties", {
                {"nodeType", {{"type", "string"},
                    {"enum", {"function", "class"}},
                    {"description", "Type of skeleton node"}}},
                {"name", {{"type", "string"},
                    {"description", "Function or class name"}}},
                {"parameters", {{"type", "array"}, {"items", {{"type", "string"}}},
                    {"description", "Parameter names (for functions)"}}},
                {"contextWidth", {{"type", "string"},
                    {"enum", {"local", "file", "project", "cross-project"}},
                    {"description", "How much context needed"}}},
                {"automatability", {{"type", "string"},
                    {"enum", {"deterministic", "template", "slm", "llm", "human"}},
                    {"description", "What kind of worker should handle this"}}},
                {"priority", {{"type", "string"},
                    {"enum", {"critical", "high", "medium", "low"}},
                    {"description", "Task priority"}}},
                {"blockedBy", {{"type", "array"}, {"items", {{"type", "string"}}},
                    {"description", "Task names this depends on"}}},
                {"methods", {{"type", "array"}, {"items", {{"type", "string"}}},
                    {"description", "Method names (for classes)"}}}
            }}, {"required", json::array({"name"})}}
        });
        toolHandlers_["whetstone_add_skeleton_node"] =
            [this](const json& args) {
                return callWhetstone("addSkeletonNode", args);
            };

        // whetstone_get_project_model
        tools_.push_back({"whetstone_get_project_model",
            "Get the skeleton summary and task list for the active buffer. "
            "Returns total/skeleton/implemented node counts plus a flat task "
            "list with routing annotations (contextWidth, automatability, "
            "priority, reviewRequired, status, dependencies).",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_project_model"] =
            [this](const json& args) {
                return callWhetstone("getProjectModel", args);
            };

        // whetstone_infer_annotations
        tools_.push_back({"whetstone_infer_annotations",
            "Auto-infer annotations on the active buffer's AST. Covers all "
            "8 annotation subjects: memory, async/exec, pure, tail-call, "
            "visibility, exception, blocking, parallel, complexity, loops. "
            "Returns suggestions with confidence scores.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_infer_annotations"] =
            [this](const json& args) {
                return callWhetstone("inferAnnotations", args);
            };
    }

