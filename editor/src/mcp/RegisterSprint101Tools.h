// Sprint 101: MCP tools
// Included inside MCPServer class body.

    void registerSprint101Tools() {
        tools_.push_back({"whetstone_get_epoch_workstreams", "whetstone_get_epoch_workstreams tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_epoch_workstreams"] = [this](const nlohmann::json& args) { return runWhetstoneGetEpochWorkstreams(args); };

        tools_.push_back({"whetstone_set_workstream_capacity", "whetstone_set_workstream_capacity tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_set_workstream_capacity"] = [this](const nlohmann::json& args) { return runWhetstoneSetWorkstreamCapacity(args); };

    }

    nlohmann::json runWhetstoneGetEpochWorkstreams(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_epoch_workstreams";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 101;
        data["step"] = 1245;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneSetWorkstreamCapacity(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_set_workstream_capacity";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 101;
        data["step"] = 1246;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

