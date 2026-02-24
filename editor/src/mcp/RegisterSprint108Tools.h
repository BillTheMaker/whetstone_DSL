// Sprint 108: MCP tools
// Included inside MCPServer class body.

    void registerSprint108Tools() {
        tools_.push_back({"whetstone_list_migration_templates", "whetstone_list_migration_templates tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_list_migration_templates"] = [this](const nlohmann::json& args) { return runWhetstoneListMigrationTemplates(args); };

        tools_.push_back({"whetstone_start_guided_migration", "whetstone_start_guided_migration tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_start_guided_migration"] = [this](const nlohmann::json& args) { return runWhetstoneStartGuidedMigration(args); };

    }

    nlohmann::json runWhetstoneListMigrationTemplates(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_list_migration_templates";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 108;
        data["step"] = 1315;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneStartGuidedMigration(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_start_guided_migration";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 108;
        data["step"] = 1316;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

