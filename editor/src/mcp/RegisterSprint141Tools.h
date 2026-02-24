// Sprint 141: MCP tools
// Included inside MCPServer class body.

    void registerSprint141Tools() {
        tools_.push_back({"whetstone_get_authoring_mode", "whetstone_get_authoring_mode tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_authoring_mode"] = [this](const nlohmann::json& args) { return runWhetstoneGetAuthoringMode(args); };

        tools_.push_back({"whetstone_set_authoring_mode", "whetstone_set_authoring_mode tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_set_authoring_mode"] = [this](const nlohmann::json& args) { return runWhetstoneSetAuthoringMode(args); };

        tools_.push_back({"whetstone_get_mode_capabilities", "whetstone_get_mode_capabilities tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_mode_capabilities"] = [this](const nlohmann::json& args) { return runWhetstoneGetModeCapabilities(args); };
    }

    nlohmann::json runWhetstoneGetAuthoringMode(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "workspaceId required"}};
        std::string workspaceId = args.value("workspaceId", "");
        if (workspaceId.empty()) return {{"success", false}, {"error", "workspaceId required"}};

        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_authoring_mode";
        out["workspaceId"] = workspaceId;
        out["status"] = "ok";

        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 141;
        data["step"] = 1653;
        data["mode"] = "text_first";
        data["available_modes"] = {"text_first", "ast_first", "hybrid"};
        data["language_default"] = "cpp";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneSetAuthoringMode(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "workspaceId required"}};
        std::string workspaceId = args.value("workspaceId", "");
        if (workspaceId.empty()) return {{"success", false}, {"error", "workspaceId required"}};
        std::string mode = args.value("mode", "text_first");
        if (mode != "text_first" && mode != "ast_first" && mode != "hybrid") {
            return {{"success", false}, {"error", "invalid mode"}};
        }

        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_set_authoring_mode";
        out["workspaceId"] = workspaceId;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 141;
        data["step"] = 1654;
        data["kind"] = "secondary";
        data["mode"] = mode;
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGetModeCapabilities(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "workspaceId required"}};
        std::string workspaceId = args.value("workspaceId", "");
        if (workspaceId.empty()) return {{"success", false}, {"error", "workspaceId required"}};

        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_mode_capabilities";
        out["workspaceId"] = workspaceId;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 141;
        data["step"] = 1655;
        data["kind"] = "tertiary";
        data["supports_text_first"] = true;
        data["supports_ast_first"] = true;
        data["supports_hybrid"] = true;
        data["default_for_cpp"] = "text_first";
        out["data"] = data;
        return out;
    }
