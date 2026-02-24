// Sprint 103: MCP tools
// Included inside MCPServer class body.

    void registerSprint103Tools() {
        tools_.push_back({"whetstone_list_verified_patterns", "whetstone_list_verified_patterns tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_list_verified_patterns"] = [this](const nlohmann::json& args) { return runWhetstoneListVerifiedPatterns(args); };

        tools_.push_back({"whetstone_apply_verified_pattern", "whetstone_apply_verified_pattern tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_apply_verified_pattern"] = [this](const nlohmann::json& args) { return runWhetstoneApplyVerifiedPattern(args); };

    }

    nlohmann::json runWhetstoneListVerifiedPatterns(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_list_verified_patterns";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 103;
        data["step"] = 1265;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneApplyVerifiedPattern(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_apply_verified_pattern";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 103;
        data["step"] = 1266;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

