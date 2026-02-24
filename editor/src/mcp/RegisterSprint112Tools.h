// Sprint 112: MCP tools
// Included inside MCPServer class body.

    void registerSprint112Tools() {
        tools_.push_back({"whetstone_snapshot_environment", "whetstone_snapshot_environment tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_snapshot_environment"] = [this](const nlohmann::json& args) { return runWhetstoneSnapshotEnvironment(args); };

        tools_.push_back({"whetstone_diff_environments", "whetstone_diff_environments tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_diff_environments"] = [this](const nlohmann::json& args) { return runWhetstoneDiffEnvironments(args); };

    }

    nlohmann::json runWhetstoneSnapshotEnvironment(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_snapshot_environment";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 112;
        data["step"] = 1356;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneDiffEnvironments(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_diff_environments";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 112;
        data["step"] = 1357;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

