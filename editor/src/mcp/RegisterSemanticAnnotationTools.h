    void registerSemanticAnnotationTools() {
        // whetstone_set_semantic_annotation
        tools_.push_back({"whetstone_set_semantic_annotation",
            "Set or update a semantic annotation on an AST node. If an "
            "annotation of the same type already exists, it is replaced. "
            "Types: intent, complexity, risk, contract, tags.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description", "Target node ID"}}},
                {"type", {{"type", "string"},
                    {"enum", {"intent", "complexity", "risk", "contract", "tags"}},
                    {"description", "Annotation type"}}},
                {"fields", {{"type", "object"},
                    {"description", "Annotation-specific fields"}}}
            }}, {"required", {"nodeId", "type", "fields"}}}
        });
        toolHandlers_["whetstone_set_semantic_annotation"] =
            [this](const json& args) {
                return callWhetstone("setSemanticAnnotation", args);
            };

        // whetstone_get_semantic_annotations
        tools_.push_back({"whetstone_get_semantic_annotations",
            "Get semantic annotations. If nodeId provided, returns "
            "annotations for that node. Otherwise returns all annotated "
            "nodes with their annotations.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description",
                     "Node ID (optional, omit for all annotated nodes)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_semantic_annotations"] =
            [this](const json& args) {
                return callWhetstone("getSemanticAnnotations", args);
            };

        // whetstone_remove_semantic_annotation
        tools_.push_back({"whetstone_remove_semantic_annotation",
            "Remove a semantic annotation of a specific type from a node.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description", "Target node ID"}}},
                {"type", {{"type", "string"},
                    {"enum", {"intent", "complexity", "risk", "contract", "tags"}},
                    {"description", "Annotation type to remove"}}}
            }}, {"required", {"nodeId", "type"}}}
        });
        toolHandlers_["whetstone_remove_semantic_annotation"] =
            [this](const json& args) {
                return callWhetstone("removeSemanticAnnotation", args);
            };

        // whetstone_get_unannotated_nodes
        tools_.push_back({"whetstone_get_unannotated_nodes",
            "Get Function/Variable nodes that lack semantic annotations. "
            "Optionally filter by annotation type to find nodes missing "
            "a specific annotation.",
            {{"type", "object"}, {"properties", {
                {"type", {{"type", "string"},
                    {"enum", {"intent", "complexity", "risk", "contract", "tags"}},
                    {"description",
                     "Filter: nodes missing this annotation type (optional)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_unannotated_nodes"] =
            [this](const json& args) {
                return callWhetstone("getUnannotatedNodes", args);
            };
    }

    // ---------------------------------------------------------------
    //  Step 286: Environment layer tools
    // ---------------------------------------------------------------
