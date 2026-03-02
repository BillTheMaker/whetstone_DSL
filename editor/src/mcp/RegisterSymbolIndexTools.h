// Step 1906: MCP wiring for whetstone_emit_symbol_index
//
// Registers:
//   whetstone_emit_symbol_index
//
// This file is #included inside the MCPServer class body.

    void registerSymbolIndexTools() {
        tools_.push_back({"whetstone_emit_symbol_index",
            "Emit a SCIP-format cross-language symbol index from a set of polyglot boundary nodes. "
            "Inserts nodes into an in-memory CrossLanguageSymbolTable and returns the SCIP index "
            "with one document per language. Supports incremental update via old/new node diff.",
            {{"type", "object"}, {"properties", {
                {"nodes", {{"type", "array"},
                    {"description", "Array of ABIBoundaryNode JSON objects (from whetstone_generate_ffi_glue output)."}}},
                {"old_nodes", {{"type", "array"},
                    {"description", "Previous node array for incremental diff (omit for full rebuild)."}}}
            }}, {"required", json::array({"nodes"})}}
        });
        toolHandlers_["whetstone_emit_symbol_index"] =
            [this](const json& args) -> json {
                return runEmitSymbolIndex(args);
            };
    }

    json runEmitSymbolIndex(const json& args) {
        if (!args.contains("nodes") || !args["nodes"].is_array())
            return {{"success", false}, {"error", "nodes array is required"}};

        // Deserialise nodes
        std::vector<whetstone::ABIBoundaryNode> nodes;
        for (const auto& j : args["nodes"]) {
            whetstone::ABIBoundaryNode n;
            if (j.contains("name"))          n.name          = j["name"].get<std::string>();
            if (j.contains("kind"))          n.kind          = j["kind"].get<std::string>();
            if (j.contains("fromComponent")) n.fromComponent = j["fromComponent"].get<std::string>();
            if (j.contains("toComponent"))   n.toComponent   = j["toComponent"].get<std::string>();
            if (j.contains("fromLanguage"))  n.fromLanguage  = j["fromLanguage"].get<std::string>();
            if (j.contains("toLanguage"))    n.toLanguage    = j["toLanguage"].get<std::string>();
            if (j.contains("signature"))     n.signature     = j["signature"];
            nodes.push_back(n);
        }

        // Incremental path
        if (args.contains("old_nodes") && args["old_nodes"].is_array()) {
            std::vector<whetstone::ABIBoundaryNode> oldNodes;
            for (const auto& j : args["old_nodes"]) {
                whetstone::ABIBoundaryNode n;
                if (j.contains("name"))          n.name          = j["name"].get<std::string>();
                if (j.contains("kind"))          n.kind          = j["kind"].get<std::string>();
                if (j.contains("fromComponent")) n.fromComponent = j["fromComponent"].get<std::string>();
                if (j.contains("toComponent"))   n.toComponent   = j["toComponent"].get<std::string>();
                if (j.contains("fromLanguage"))  n.fromLanguage  = j["fromLanguage"].get<std::string>();
                if (j.contains("toLanguage"))    n.toLanguage    = j["toLanguage"].get<std::string>();
                oldNodes.push_back(n);
            }
            whetstone::CrossLanguageSymbolTable table;
            for (auto& n : oldNodes) table.insert(n);
            auto diff = whetstone::SymbolIndexOrchestrator::update(table, oldNodes, nodes);
            auto scip = whetstone::SCIPEmitter::emit(nodes);
            return {{"success", true}, {"mode", "incremental"},
                    {"added", diff.added}, {"removed", diff.removed},
                    {"unchanged", diff.unchanged}, {"scip_index", scip}};
        }

        // Full rebuild
        whetstone::CrossLanguageSymbolTable table;
        auto result = whetstone::SymbolIndexOrchestrator::orchestrate(nodes, table);
        result["mode"] = "full";
        return result;
    }
