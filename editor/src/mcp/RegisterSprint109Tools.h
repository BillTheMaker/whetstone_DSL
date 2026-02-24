// Sprint 109: MCP tools
// Included inside MCPServer class body.

    void registerSprint109Tools() {
        tools_.push_back({"whetstone_get_debt_inventory", "whetstone_get_debt_inventory tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_debt_inventory"] = [this](const nlohmann::json& args) { return runWhetstoneGetDebtInventory(args); };

        tools_.push_back({"whetstone_plan_debt_burndown", "whetstone_plan_debt_burndown tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_plan_debt_burndown"] = [this](const nlohmann::json& args) { return runWhetstonePlanDebtBurndown(args); };

    }

    nlohmann::json runWhetstoneGetDebtInventory(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_debt_inventory";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 109;
        data["step"] = 1325;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstonePlanDebtBurndown(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_plan_debt_burndown";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 109;
        data["step"] = 1326;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

