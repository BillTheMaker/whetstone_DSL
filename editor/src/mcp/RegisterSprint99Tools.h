// Sprint 99: MCP tools
// Included inside MCPServer class body.

    void registerSprint99Tools() {
        tools_.push_back({"whetstone_run_continuity_drill", "whetstone_run_continuity_drill tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_run_continuity_drill"] = [this](const nlohmann::json& args) { return runWhetstoneRunContinuityDrill(args); };

        tools_.push_back({"whetstone_get_recovery_readiness", "whetstone_get_recovery_readiness tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_recovery_readiness"] = [this](const nlohmann::json& args) { return runWhetstoneGetRecoveryReadiness(args); };

    }

    nlohmann::json runWhetstoneRunContinuityDrill(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_run_continuity_drill";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 99;
        data["step"] = 1225;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGetRecoveryReadiness(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_recovery_readiness";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 99;
        data["step"] = 1226;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

