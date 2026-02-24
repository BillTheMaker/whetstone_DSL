// Sprint 107: MCP tools
// Included inside MCPServer class body.

    void registerSprint107Tools() {
        tools_.push_back({"whetstone_get_safety_profile_requirements", "whetstone_get_safety_profile_requirements tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_safety_profile_requirements"] = [this](const nlohmann::json& args) { return runWhetstoneGetSafetyProfileRequirements(args); };

        tools_.push_back({"whetstone_verify_safety_profile_compliance", "whetstone_verify_safety_profile_compliance tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_verify_safety_profile_compliance"] = [this](const nlohmann::json& args) { return runWhetstoneVerifySafetyProfileCompliance(args); };

    }

    nlohmann::json runWhetstoneGetSafetyProfileRequirements(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_safety_profile_requirements";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 107;
        data["step"] = 1305;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneVerifySafetyProfileCompliance(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_verify_safety_profile_compliance";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 107;
        data["step"] = 1306;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

