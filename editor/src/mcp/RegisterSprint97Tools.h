// Sprint 97: MCP tools
// Included inside MCPServer class body.

    void registerSprint97Tools() {
        tools_.push_back({"whetstone_generate_patch_candidates", "whetstone_generate_patch_candidates tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_generate_patch_candidates"] = [this](const nlohmann::json& args) { return runWhetstoneGeneratePatchCandidates(args); };

        tools_.push_back({"whetstone_validate_patch_candidate", "whetstone_validate_patch_candidate tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_validate_patch_candidate"] = [this](const nlohmann::json& args) { return runWhetstoneValidatePatchCandidate(args); };

    }

    nlohmann::json runWhetstoneGeneratePatchCandidates(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_generate_patch_candidates";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 97;
        data["step"] = 1205;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneValidatePatchCandidate(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_validate_patch_candidate";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 97;
        data["step"] = 1206;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

