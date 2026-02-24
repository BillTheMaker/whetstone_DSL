// Sprint 113: MCP tools
// Included inside MCPServer class body.

    void registerSprint113Tools() {
        tools_.push_back({"whetstone_parse_build_output", "whetstone_parse_build_output tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_parse_build_output"] = [this](const nlohmann::json& args) { return runWhetstoneParseBuildOutput(args); };

        tools_.push_back({"whetstone_suggest_build_fix", "whetstone_suggest_build_fix tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_suggest_build_fix"] = [this](const nlohmann::json& args) { return runWhetstoneSuggestBuildFix(args); };

        tools_.push_back({"whetstone_run_build_iteration", "whetstone_run_build_iteration tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_run_build_iteration"] = [this](const nlohmann::json& args) { return runWhetstoneRunBuildIteration(args); };

    }

    nlohmann::json runWhetstoneParseBuildOutput(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_parse_build_output";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 113;
        data["step"] = 1365;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneSuggestBuildFix(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_suggest_build_fix";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 113;
        data["step"] = 1366;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneRunBuildIteration(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_run_build_iteration";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 113;
        data["step"] = 1367;
        data["kind"] = "tertiary";
        out["data"] = data;
        return out;
    }

