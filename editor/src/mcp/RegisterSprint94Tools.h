// Sprint 94: MCP tools
// Included inside MCPServer class body.

    void registerSprint94Tools() {
        tools_.push_back({"whetstone_get_execution_attestation", "whetstone_get_execution_attestation tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_execution_attestation"] = [this](const nlohmann::json& args) { return runWhetstoneGetExecutionAttestation(args); };

        tools_.push_back({"whetstone_set_zero_trust_policy", "whetstone_set_zero_trust_policy tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_set_zero_trust_policy"] = [this](const nlohmann::json& args) { return runWhetstoneSetZeroTrustPolicy(args); };

    }

    nlohmann::json runWhetstoneGetExecutionAttestation(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_execution_attestation";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 94;
        data["step"] = 1175;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneSetZeroTrustPolicy(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_set_zero_trust_policy";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 94;
        data["step"] = 1176;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

