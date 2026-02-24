// Sprint 110: MCP tools
// Included inside MCPServer class body.

    void registerSprint110Tools() {
        tools_.push_back({"whetstone_get_epoch_block_status", "whetstone_get_epoch_block_status tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_epoch_block_status"] = [this](const nlohmann::json& args) { return runWhetstoneGetEpochBlockStatus(args); };

        tools_.push_back({"whetstone_publish_next_block_plan", "whetstone_publish_next_block_plan tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_publish_next_block_plan"] = [this](const nlohmann::json& args) { return runWhetstonePublishNextBlockPlan(args); };

    }

    nlohmann::json runWhetstoneGetEpochBlockStatus(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_epoch_block_status";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 110;
        data["step"] = 1335;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstonePublishNextBlockPlan(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_publish_next_block_plan";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 110;
        data["step"] = 1336;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

