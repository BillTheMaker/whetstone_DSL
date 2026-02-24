// Sprint 93: MCP tools
// Included inside MCPServer class body.

    void registerSprint93Tools() {
        tools_.push_back({"whetstone_get_portfolio_economics", "whetstone_get_portfolio_economics tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_portfolio_economics"] = [this](const nlohmann::json& args) { return runWhetstoneGetPortfolioEconomics(args); };

        tools_.push_back({"whetstone_plan_budget_allocation", "whetstone_plan_budget_allocation tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_plan_budget_allocation"] = [this](const nlohmann::json& args) { return runWhetstonePlanBudgetAllocation(args); };

    }

    nlohmann::json runWhetstoneGetPortfolioEconomics(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_portfolio_economics";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 93;
        data["step"] = 1165;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstonePlanBudgetAllocation(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_plan_budget_allocation";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 93;
        data["step"] = 1166;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

