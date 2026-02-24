// Sprint 105: MCP tools
// Included inside MCPServer class body.

    void registerSprint105Tools() {
        tools_.push_back({"whetstone_propose_policy_tuning", "whetstone_propose_policy_tuning tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_propose_policy_tuning"] = [this](const nlohmann::json& args) { return runWhetstoneProposePolicyTuning(args); };

        tools_.push_back({"whetstone_validate_policy_tuning", "whetstone_validate_policy_tuning tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_validate_policy_tuning"] = [this](const nlohmann::json& args) { return runWhetstoneValidatePolicyTuning(args); };

    }

    nlohmann::json runWhetstoneProposePolicyTuning(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_propose_policy_tuning";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 105;
        data["step"] = 1285;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneValidatePolicyTuning(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_validate_policy_tuning";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 105;
        data["step"] = 1286;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

