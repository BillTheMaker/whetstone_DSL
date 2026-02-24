// Sprint 118: MCP tools
// Included inside MCPServer class body.

    void registerSprint118Tools() {
        tools_.push_back({"whetstone_capture_distributed_failure", "whetstone_capture_distributed_failure tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_capture_distributed_failure"] = [this](const nlohmann::json& args) { return runWhetstoneCaptureDistributedFailure(args); };

        tools_.push_back({"whetstone_list_distributed_failures", "whetstone_list_distributed_failures tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_list_distributed_failures"] = [this](const nlohmann::json& args) { return runWhetstoneListDistributedFailures(args); };

        tools_.push_back({"whetstone_get_distributed_failure_bundle", "whetstone_get_distributed_failure_bundle tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_distributed_failure_bundle"] = [this](const nlohmann::json& args) { return runWhetstoneGetDistributedFailureBundle(args); };

    }

    nlohmann::json runWhetstoneCaptureDistributedFailure(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_capture_distributed_failure";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 118;
        data["step"] = 1423;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneListDistributedFailures(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_list_distributed_failures";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 118;
        data["step"] = 1424;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGetDistributedFailureBundle(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_distributed_failure_bundle";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 118;
        data["step"] = 1425;
        data["kind"] = "tertiary";
        out["data"] = data;
        return out;
    }

