// Step 835: MCP tools for graduation/transpile support matrix.
// Included inside MCPServer class body.

    void registerGraduationTools() {
        // whetstone_get_transpile_support_matrix
        tools_.push_back({"whetstone_get_transpile_support_matrix",
            "Get the transpilation support matrix, optionally filtered by tier.",
            {{"type","object"},{"properties",{
                {"tier",{{"type","string"},{"description","Filter by tier: stable, beta, experimental, or all (default)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_transpile_support_matrix"] =
            [this](const nlohmann::json& args) { return runGetTranspileSupportMatrix(args); };
    }

    nlohmann::json runGetTranspileSupportMatrix(const nlohmann::json& args) {
        std::string tier = "all";
        if (args.contains("tier") && args["tier"].is_string())
            tier = args["tier"].get<std::string>();

        // Hardcoded matrix of 5 language pairs
        std::vector<nlohmann::json> allPairs = {
            {{"source","python"},{"target","cpp"},{"tier","stable"}},
            {{"source","rust"},{"target","cpp"},{"tier","stable"}},
            {{"source","python"},{"target","js"},{"tier","beta"}},
            {{"source","cpp"},{"target","rust"},{"tier","beta"}},
            {{"source","js"},{"target","ts"},{"tier","experimental"}}
        };

        std::vector<nlohmann::json> filtered;
        for (auto& p : allPairs) {
            if (tier == "all" || p["tier"] == tier)
                filtered.push_back(p);
        }

        nlohmann::json pairsArr = nlohmann::json::array();
        for (auto& p : filtered) pairsArr.push_back(p);
        return {{"success",true},{"pairs",pairsArr},{"total",(int)filtered.size()}};
    }
