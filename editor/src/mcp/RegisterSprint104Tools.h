// Sprint 104: MCP tools
// Included inside MCPServer class body.

    void registerSprint104Tools() {
        tools_.push_back({"whetstone_plan_polyglot_migration", "whetstone_plan_polyglot_migration tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_plan_polyglot_migration"] = [this](const nlohmann::json& args) { return runWhetstonePlanPolyglotMigration(args); };

        tools_.push_back({"whetstone_get_polyglot_cutover_readiness", "whetstone_get_polyglot_cutover_readiness tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_polyglot_cutover_readiness"] = [this](const nlohmann::json& args) { return runWhetstoneGetPolyglotCutoverReadiness(args); };

    }

    nlohmann::json runWhetstonePlanPolyglotMigration(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_plan_polyglot_migration";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 104;
        data["step"] = 1275;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGetPolyglotCutoverReadiness(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_polyglot_cutover_readiness";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 104;
        data["step"] = 1276;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

