// Sprint 116: MCP tools
// Included inside MCPServer class body.

    void registerSprint116Tools() {
        tools_.push_back({"whetstone_start_feedback_loop", "whetstone_start_feedback_loop tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_start_feedback_loop"] = [this](const nlohmann::json& args) { return runWhetstoneStartFeedbackLoop(args); };

        tools_.push_back({"whetstone_step_feedback_loop", "whetstone_step_feedback_loop tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_step_feedback_loop"] = [this](const nlohmann::json& args) { return runWhetstoneStepFeedbackLoop(args); };

        tools_.push_back({"whetstone_run_feedback_loop", "whetstone_run_feedback_loop tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_run_feedback_loop"] = [this](const nlohmann::json& args) { return runWhetstoneRunFeedbackLoop(args); };

    }

    nlohmann::json runWhetstoneStartFeedbackLoop(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_start_feedback_loop";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 116;
        data["step"] = 1395;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneStepFeedbackLoop(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_step_feedback_loop";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 116;
        data["step"] = 1396;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneRunFeedbackLoop(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_run_feedback_loop";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 116;
        data["step"] = 1397;
        data["kind"] = "tertiary";
        out["data"] = data;
        return out;
    }

