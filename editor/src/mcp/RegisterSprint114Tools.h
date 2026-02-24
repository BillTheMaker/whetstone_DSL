// Sprint 114: MCP tools
// Included inside MCPServer class body.

    void registerSprint114Tools() {
        tools_.push_back({"whetstone_parse_test_output", "whetstone_parse_test_output tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_parse_test_output"] = [this](const nlohmann::json& args) { return runWhetstoneParseTestOutput(args); };

        tools_.push_back({"whetstone_suggest_test_fix", "whetstone_suggest_test_fix tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_suggest_test_fix"] = [this](const nlohmann::json& args) { return runWhetstoneSuggestTestFix(args); };

        tools_.push_back({"whetstone_run_test_iteration", "whetstone_run_test_iteration tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_run_test_iteration"] = [this](const nlohmann::json& args) { return runWhetstoneRunTestIteration(args); };

    }

    nlohmann::json runWhetstoneParseTestOutput(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_parse_test_output";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 114;
        data["step"] = 1375;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneSuggestTestFix(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_suggest_test_fix";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 114;
        data["step"] = 1376;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneRunTestIteration(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_run_test_iteration";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 114;
        data["step"] = 1377;
        data["kind"] = "tertiary";
        out["data"] = data;
        return out;
    }

