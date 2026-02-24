// Sprint 95: MCP tools
// Included inside MCPServer class body.

    void registerSprint95Tools() {
        tools_.push_back({"whetstone_get_review_load_status", "whetstone_get_review_load_status tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_review_load_status"] = [this](const nlohmann::json& args) { return runWhetstoneGetReviewLoadStatus(args); };

        tools_.push_back({"whetstone_optimize_review_queue", "whetstone_optimize_review_queue tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_optimize_review_queue"] = [this](const nlohmann::json& args) { return runWhetstoneOptimizeReviewQueue(args); };

    }

    nlohmann::json runWhetstoneGetReviewLoadStatus(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_review_load_status";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 95;
        data["step"] = 1185;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneOptimizeReviewQueue(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_optimize_review_queue";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 95;
        data["step"] = 1186;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

