// Sprint 145: MCP tools
// Included inside MCPServer class body.

    void registerSprint145Tools() {
        tools_.push_back({"whetstone_run_cpp_constructive_step", "whetstone_run_cpp_constructive_step tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_run_cpp_constructive_step"] = [this](const nlohmann::json& args) { return runWhetstoneRunCppConstructiveStep(args); };

        tools_.push_back({"whetstone_get_cpp_constructive_status", "whetstone_get_cpp_constructive_status tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_cpp_constructive_status"] = [this](const nlohmann::json& args) { return runWhetstoneGetCppConstructiveStatus(args); };

        tools_.push_back({"whetstone_run_cpp_constructive_loop", "whetstone_run_cpp_constructive_loop tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_run_cpp_constructive_loop"] = [this](const nlohmann::json& args) { return runWhetstoneRunCppConstructiveLoop(args); };
    }

    nlohmann::json runWhetstoneRunCppConstructiveStep(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_run_cpp_constructive_step";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 145}, {"step", 1693}, {"kind", "primary"}};
        return out;
    }

    nlohmann::json runWhetstoneGetCppConstructiveStatus(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_cpp_constructive_status";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 145}, {"step", 1694}, {"kind", "secondary"}};
        return out;
    }

    nlohmann::json runWhetstoneRunCppConstructiveLoop(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_run_cpp_constructive_loop";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 145}, {"step", 1695}, {"kind", "tertiary"}};
        return out;
    }
