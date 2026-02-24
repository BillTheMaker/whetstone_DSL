// Sprint 102: MCP tools
// Included inside MCPServer class body.

    void registerSprint102Tools() {
        tools_.push_back({"whetstone_plan_swarm_maintenance", "whetstone_plan_swarm_maintenance tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_plan_swarm_maintenance"] = [this](const nlohmann::json& args) { return runWhetstonePlanSwarmMaintenance(args); };

        tools_.push_back({"whetstone_get_swarm_maintenance_status", "whetstone_get_swarm_maintenance_status tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_swarm_maintenance_status"] = [this](const nlohmann::json& args) { return runWhetstoneGetSwarmMaintenanceStatus(args); };

    }

    nlohmann::json runWhetstonePlanSwarmMaintenance(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_plan_swarm_maintenance";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 102;
        data["step"] = 1255;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneGetSwarmMaintenanceStatus(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_swarm_maintenance_status";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 102;
        data["step"] = 1256;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

