// Sprint 111: MCP tools
// Included inside MCPServer class body.

    void registerSprint111Tools() {
        tools_.push_back({"whetstone_start_iteration_session", "whetstone_start_iteration_session tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_start_iteration_session"] = [this](const nlohmann::json& args) { return runWhetstoneStartIterationSession(args); };

        tools_.push_back({"whetstone_record_attempt", "whetstone_record_attempt tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_record_attempt"] = [this](const nlohmann::json& args) { return runWhetstoneRecordAttempt(args); };

        tools_.push_back({"whetstone_get_session_state", "whetstone_get_session_state tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_session_state"] = [this](const nlohmann::json& args) { return runWhetstoneGetSessionState(args); };

    }

    nlohmann::json runWhetstoneStartIterationSession(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_start_iteration_session";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 111;
        data["step"] = 1345;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneRecordAttempt(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_record_attempt";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 111;
        data["step"] = 1346;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGetSessionState(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_session_state";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 111;
        data["step"] = 1347;
        data["kind"] = "tertiary";
        out["data"] = data;
        return out;
    }

