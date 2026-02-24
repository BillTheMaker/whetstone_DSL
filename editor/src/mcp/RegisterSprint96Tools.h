// Sprint 96: MCP tools
// Included inside MCPServer class body.

    void registerSprint96Tools() {
        tools_.push_back({"whetstone_query_transpile_graph", "whetstone_query_transpile_graph tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_query_transpile_graph"] = [this](const nlohmann::json& args) { return runWhetstoneQueryTranspileGraph(args); };

        tools_.push_back({"whetstone_get_related_migration_patterns", "whetstone_get_related_migration_patterns tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_related_migration_patterns"] = [this](const nlohmann::json& args) { return runWhetstoneGetRelatedMigrationPatterns(args); };

    }

    nlohmann::json runWhetstoneQueryTranspileGraph(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_query_transpile_graph";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 96;
        data["step"] = 1195;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGetRelatedMigrationPatterns(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_related_migration_patterns";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 96;
        data["step"] = 1196;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

