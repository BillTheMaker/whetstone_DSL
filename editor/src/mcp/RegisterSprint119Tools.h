// Sprint 119: MCP tools
// Included inside MCPServer class body.

    void registerSprint119Tools() {
        tools_.push_back({"whetstone_cluster_distributed_failures", "whetstone_cluster_distributed_failures tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_cluster_distributed_failures"] = [this](const nlohmann::json& args) { return runWhetstoneClusterDistributedFailures(args); };

        tools_.push_back({"whetstone_rank_failure_triage_actions", "whetstone_rank_failure_triage_actions tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_rank_failure_triage_actions"] = [this](const nlohmann::json& args) { return runWhetstoneRankFailureTriageActions(args); };

        tools_.push_back({"whetstone_get_distributed_triage_queue", "whetstone_get_distributed_triage_queue tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_distributed_triage_queue"] = [this](const nlohmann::json& args) { return runWhetstoneGetDistributedTriageQueue(args); };

    }

    nlohmann::json runWhetstoneClusterDistributedFailures(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_cluster_distributed_failures";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 119;
        data["step"] = 1433;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneRankFailureTriageActions(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_rank_failure_triage_actions";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 119;
        data["step"] = 1434;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGetDistributedTriageQueue(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_distributed_triage_queue";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 119;
        data["step"] = 1435;
        data["kind"] = "tertiary";
        out["data"] = data;
        return out;
    }

