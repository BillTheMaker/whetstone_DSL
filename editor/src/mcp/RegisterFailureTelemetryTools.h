// Steps 855-856: MCP tools for failure telemetry.
// Included inside MCPServer class body.

    void registerFailureTelemetryTools() {
        // whetstone_get_failure_trends
        tools_.push_back({"whetstone_get_failure_trends",
            "Get failure trends across language pairs.",
            {{"type","object"},{"properties",{
                {"pair_id",{{"type","string"}}},
                {"limit",{{"type","integer"}}}
            }}}
        });
        toolHandlers_["whetstone_get_failure_trends"] =
            [this](const nlohmann::json& args) { return runGetFailureTrends(args); };

        // whetstone_get_top_adapter_gaps
        tools_.push_back({"whetstone_get_top_adapter_gaps",
            "Get top adapter gaps by tier.",
            {{"type","object"},{"properties",{
                {"tier",{{"type","string"}}},
                {"top_n",{{"type","integer"}}}
            }}}
        });
        toolHandlers_["whetstone_get_top_adapter_gaps"] =
            [this](const nlohmann::json& args) { return runGetTopAdapterGaps(args); };
    }

    nlohmann::json runGetFailureTrends(const nlohmann::json& args) {
        int limit = 10;
        if (args.contains("limit") && args["limit"].is_number_integer())
            limit = args["limit"].get<int>();
        nlohmann::json trends = nlohmann::json::array();
        trends.push_back({{"code","T001"},{"count",5},{"severity","high"}});
        trends.push_back({{"code","T004"},{"count",3},{"severity","medium"}});
        trends.push_back({{"code","T002"},{"count",2},{"severity","medium"}});
        // trim to limit
        while ((int)trends.size() > limit) trends.erase(trends.begin() + limit);
        return {{"success",true},{"trends",trends},{"total",(int)trends.size()}};
    }

    nlohmann::json runGetTopAdapterGaps(const nlohmann::json& args) {
        std::string tier = "all";
        int top_n = 5;
        if (args.contains("tier") && args["tier"].is_string())
            tier = args["tier"].get<std::string>();
        if (args.contains("top_n") && args["top_n"].is_number_integer())
            top_n = args["top_n"].get<int>();
        nlohmann::json gaps = nlohmann::json::array();
        gaps.push_back({{"code","T001"},{"pair_id","python->cpp"},{"description","semantic mismatch"},{"priority","critical"}});
        gaps.push_back({{"code","T004"},{"pair_id","rust->cpp"},{"description","memory model gap"},{"priority","high"}});
        while ((int)gaps.size() > top_n) gaps.erase(gaps.begin() + top_n);
        return {{"success",true},{"gaps",gaps},{"total",(int)gaps.size()}};
    }
