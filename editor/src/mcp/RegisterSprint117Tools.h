// Sprint 117: MCP tools
// Included inside MCPServer class body.

    void registerSprint117Tools() {
        tools_.push_back({"whetstone_configure_hivemind", "whetstone_configure_hivemind tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_configure_hivemind"] = [this](const nlohmann::json& args) { return runWhetstoneConfigureHivemind(args); };

        tools_.push_back({"whetstone_submit_iteration_job", "whetstone_submit_iteration_job tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_submit_iteration_job"] = [this](const nlohmann::json& args) { return runWhetstoneSubmitIterationJob(args); };

        tools_.push_back({"whetstone_poll_iteration_job", "whetstone_poll_iteration_job tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_poll_iteration_job"] = [this](const nlohmann::json& args) { return runWhetstonePollIterationJob(args); };

    }

    nlohmann::json runWhetstoneConfigureHivemind(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_configure_hivemind";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 117;
        data["step"] = 1404;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneSubmitIterationJob(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_submit_iteration_job";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 117;
        data["step"] = 1405;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstonePollIterationJob(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_poll_iteration_job";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 117;
        data["step"] = 1406;
        data["kind"] = "tertiary";
        out["data"] = data;
        return out;
    }

