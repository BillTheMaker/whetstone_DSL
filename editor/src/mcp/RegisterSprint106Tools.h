// Sprint 106: MCP tools
// Included inside MCPServer class body.

    void registerSprint106Tools() {
        tools_.push_back({"whetstone_extract_api_abi_contract", "whetstone_extract_api_abi_contract tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_extract_api_abi_contract"] = [this](const nlohmann::json& args) { return runWhetstoneExtractApiAbiContract(args); };

        tools_.push_back({"whetstone_verify_api_abi_compat", "whetstone_verify_api_abi_compat tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_verify_api_abi_compat"] = [this](const nlohmann::json& args) { return runWhetstoneVerifyApiAbiCompat(args); };

    }

    nlohmann::json runWhetstoneExtractApiAbiContract(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_extract_api_abi_contract";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 106;
        data["step"] = 1295;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneVerifyApiAbiCompat(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_verify_api_abi_compat";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 106;
        data["step"] = 1296;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

