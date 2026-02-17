    void registerASTTools() {
        // whetstone_get_ast
        tools_.push_back({"whetstone_get_ast",
            "Get the current AST of the active buffer. Set compact=true for "
            "a token-efficient flat list of {id, type, name, line, children}. "
            "Full mode returns complete tree with properties and spans.",
            {{"type", "object"}, {"properties", {
                {"compact", {{"type", "boolean"}, {"description", "Compact mode: flat list with minimal fields (default false)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_ast"] = [this](const json& args) {
            return callWhetstone("getAST", args);
        };

        // whetstone_mutate
        tools_.push_back({"whetstone_mutate",
            "Apply a single mutation to the AST. Supports setProperty (rename/change), "
            "updateNode (bulk property update), deleteNode, and insertNode operations.",
            {{"type", "object"}, {"properties", {
                {"type", {{"type", "string"}, {"enum", {"setProperty", "updateNode", "deleteNode", "insertNode"}}, {"description", "Mutation type"}}},
                {"nodeId", {{"type", "string"}, {"description", "Target node ID (for setProperty, updateNode, deleteNode)"}}},
                {"property", {{"type", "string"}, {"description", "Property name (for setProperty)"}}},
                {"value", {{"type", "string"}, {"description", "New value (for setProperty)"}}},
                {"parentId", {{"type", "string"}, {"description", "Parent node ID (for insertNode)"}}},
                {"role", {{"type", "string"}, {"description", "Child role (for insertNode)"}}},
                {"node", {{"type", "object"}, {"description", "Node to insert (for insertNode)"}}}
            }}, {"required", {"type"}}}
        });
        toolHandlers_["whetstone_mutate"] = [this](const json& args) {
            return callWhetstone("applyMutation", args);
        };

        // whetstone_batch_mutate
        tools_.push_back({"whetstone_batch_mutate",
            "Apply multiple mutations atomically. All mutations succeed or all are rolled back. "
            "Each mutation has: type (setProperty|deleteNode|insertNode), nodeId, property, value, etc.",
            {{"type", "object"}, {"properties", {
                {"mutations", {{"type", "array"}, {"items", {{"type", "object"}}}, {"description", "Array of mutation objects"}}}
            }}, {"required", {"mutations"}}}
        });
        toolHandlers_["whetstone_batch_mutate"] = [this](const json& args) {
            return callWhetstone("applyBatch", args);
        };

        // whetstone_get_scope
        tools_.push_back({"whetstone_get_scope",
            "Get all symbols (variables, functions, parameters) visible at a given AST node. "
            "Walks up the scope chain to collect all accessible identifiers.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"}, {"description", "Node ID to query scope from"}}}
            }}, {"required", {"nodeId"}}}
        });
        toolHandlers_["whetstone_get_scope"] = [this](const json& args) {
            return callWhetstone("getInScopeSymbols", args);
        };

        // whetstone_get_call_hierarchy
        tools_.push_back({"whetstone_get_call_hierarchy",
            "Get the call hierarchy for a function: which functions call it (callers) "
            "and which functions it calls (callees).",
            {{"type", "object"}, {"properties", {
                {"functionId", {{"type", "string"}, {"description", "Function node ID"}}}
            }}, {"required", {"functionId"}}}
        });
        toolHandlers_["whetstone_get_call_hierarchy"] = [this](const json& args) {
            return callWhetstone("getCallHierarchy", args);
        };

        // whetstone_get_ast_subtree
        tools_.push_back({"whetstone_get_ast_subtree",
            "Get only the subtree rooted at a specific node ID. Returns full "
            "node detail for just that subtree, saving tokens vs full AST.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"}, {"description", "Root node ID for the subtree"}}}
            }}, {"required", {"nodeId"}}}
        });
        toolHandlers_["whetstone_get_ast_subtree"] = [this](const json& args) {
            return callWhetstone("getASTSubtree", args);
        };

        // whetstone_get_ast_diff
        tools_.push_back({"whetstone_get_ast_diff",
            "Get only the AST nodes that changed since a given version. "
            "Use the version number from a previous getAST or mutation response.",
            {{"type", "object"}, {"properties", {
                {"sinceVersion", {{"type", "integer"}, {"description", "Version number to diff against (from previous response)"}}}
            }}, {"required", {"sinceVersion"}}}
        });
        toolHandlers_["whetstone_get_ast_diff"] = [this](const json& args) {
            return callWhetstone("getASTDiff", args);
        };
    }

    // ---------------------------------------------------------------
    //  Step 209: Register annotation and generation tools
    // ---------------------------------------------------------------
