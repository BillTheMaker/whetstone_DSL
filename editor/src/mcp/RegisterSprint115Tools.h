// Sprint 115: MCP tools
// Included inside MCPServer class body.

    void registerSprint115Tools() {
        tools_.push_back({"whetstone_derive_requirements", "whetstone_derive_requirements tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_derive_requirements"] = [this](const nlohmann::json& args) { return runWhetstoneDeriveRequirements(args); };

        tools_.push_back({"whetstone_generate_setup_script", "whetstone_generate_setup_script tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_generate_setup_script"] = [this](const nlohmann::json& args) { return runWhetstoneGenerateSetupScript(args); };

        tools_.push_back({"whetstone_verify_requirements", "whetstone_verify_requirements tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_verify_requirements"] = [this](const nlohmann::json& args) { return runWhetstoneVerifyRequirements(args); };

    }

    nlohmann::json runWhetstoneDeriveRequirements(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_derive_requirements";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 115;
        data["step"] = 1385;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGenerateSetupScript(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_generate_setup_script";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 115;
        data["step"] = 1386;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneVerifyRequirements(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_verify_requirements";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 115;
        data["step"] = 1387;
        data["kind"] = "tertiary";
        out["data"] = data;
        return out;
    }

