// Sprint 64: Cost-Aware Transpilation Planning MCP tools (Steps 875-876)
// Included inside MCPServer class body.

    void registerCostPlanningTools() {
        tools_.push_back({"whetstone_plan_transpilation_run",
            "Generate cost-aware plan alternatives for a transpilation run.",
            {{"type", "object"}, {"properties", {
                {"pair_id", {{"type", "string"}}},
                {"lines_of_code", {{"type", "integer"}}},
                {"ambiguity_score", {{"type", "number"}}},
                {"budget_limit", {{"type", "number"}}}
            }}, {"required", nlohmann::json::array({"pair_id"})}}
        });
        toolHandlers_["whetstone_plan_transpilation_run"] =
            [this](const nlohmann::json& args) { return runPlanTranspilationRun(args); };

        tools_.push_back({"whetstone_estimate_porting_cost",
            "Estimate porting cost for a pair from LOC and ambiguity score.",
            {{"type", "object"}, {"properties", {
                {"pair_id", {{"type", "string"}}},
                {"lines_of_code", {{"type", "integer"}}},
                {"ambiguity_score", {{"type", "number"}}}
            }}, {"required", nlohmann::json::array({"pair_id"})}}
        });
        toolHandlers_["whetstone_estimate_porting_cost"] =
            [this](const nlohmann::json& args) { return runEstimatePortingCost(args); };
    }

    nlohmann::json runPlanTranspilationRun(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "pair_id required"}};
        std::string pairId = args.value("pair_id", "");
        if (pairId.empty()) return {{"success", false}, {"error", "pair_id required"}};
        int loc = args.value("lines_of_code", 100);
        float ambiguity = args.value("ambiguity_score", 0.3f);
        float budgetLimit = args.value("budget_limit", 1.0f);
        auto costEst = PortingCostModel::estimate(pairId, loc, ambiguity);
        auto plans = MultiPlanAlternativeGenerator::generate(pairId, costEst.totalCost);
        BudgetPolicy pol{budgetLimit, true};
        nlohmann::json planArr = nlohmann::json::array();
        for (const auto& plan : plans) {
            auto enforcement = BudgetPolicyEnforcer::enforce(pairId, plan.estimatedCost, pol);
            auto pj = MultiPlanAlternativeGenerator::toJson(plan);
            pj["budget_decision"] = enforcement.decision;
            planArr.push_back(pj);
        }
        return {{"success", true}, {"pair_id", pairId},
                {"base_cost_tier", costEst.tier},
                {"plans", planArr},
                {"plan_count", (int)plans.size()}};
    }

    nlohmann::json runEstimatePortingCost(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "pair_id required"}};
        std::string pairId = args.value("pair_id", "");
        if (pairId.empty()) return {{"success", false}, {"error", "pair_id required"}};
        int loc = args.value("lines_of_code", 100);
        float ambiguity = args.value("ambiguity_score", 0.3f);
        auto est = PortingCostModel::estimate(pairId, loc, ambiguity);
        auto j = PortingCostModel::toJson(est);
        j["success"] = true;
        return j;
    }
