// Sprint 100: MCP tools
// Included inside MCPServer class body.

    void registerSprint100Tools() {
        tools_.push_back({"whetstone_get_century_status", "whetstone_get_century_status tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_century_status"] = [this](const nlohmann::json& args) { return runWhetstoneGetCenturyStatus(args); };

        tools_.push_back({"whetstone_publish_next_epoch_plan", "whetstone_publish_next_epoch_plan tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_publish_next_epoch_plan"] = [this](const nlohmann::json& args) { return runWhetstonePublishNextEpochPlan(args); };

    }

    nlohmann::json runWhetstoneGetCenturyStatus(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_century_status";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 100;
        data["step"] = 1235;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstonePublishNextEpochPlan(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_publish_next_epoch_plan";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 100;
        data["step"] = 1236;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

