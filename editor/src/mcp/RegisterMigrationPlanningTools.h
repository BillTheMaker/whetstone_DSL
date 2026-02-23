// Sprint 67: Migration Planning MCP tools (Steps 907)
// Included inside MCPServer class body.

    void registerMigrationPlanningTools() {
        tools_.push_back({"whetstone_plan_migration_path",
            "Generate a cross-runtime migration path plan with step sequence and feasibility.",
            {{"type", "object"}, {"properties", {
                {"pair_id",        {{"type", "string"}}},
                {"source_runtime", {{"type", "string"}}},
                {"target_runtime", {{"type", "string"}}},
                {"risk_score",     {{"type", "number"}}}
            }}, {"required", nlohmann::json::array({"pair_id", "source_runtime", "target_runtime"})}}
        });
        toolHandlers_["whetstone_plan_migration_path"] =
            [this](const nlohmann::json& args) { return runPlanMigrationPath(args); };

        tools_.push_back({"whetstone_track_migration_progress",
            "Track migration progress for a language pair (start/advance/complete/fail/get).",
            {{"type", "object"}, {"properties", {
                {"pair_id",     {{"type", "string"}}},
                {"action",      {{"type", "string"}}},
                {"total_steps", {{"type", "integer"}}}
            }}, {"required", nlohmann::json::array({"pair_id", "action"})}}
        });
        toolHandlers_["whetstone_track_migration_progress"] =
            [this](const nlohmann::json& args) { return runTrackMigrationProgress(args); };
    }

    nlohmann::json runPlanMigrationPath(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "pair_id required"}};
        std::string pairId       = args.value("pair_id", "");
        std::string sourceRuntime = args.value("source_runtime", "");
        std::string targetRuntime = args.value("target_runtime", "");
        if (pairId.empty())        return {{"success", false}, {"error", "pair_id required"}};
        if (sourceRuntime.empty()) return {{"success", false}, {"error", "source_runtime required"}};
        if (targetRuntime.empty()) return {{"success", false}, {"error", "target_runtime required"}};
        float riskScore = args.value("risk_score", 0.2f);

        // Generate step sequence
        auto seqSteps = MigrationStepSequencer::sequence(pairId, sourceRuntime, targetRuntime);
        std::vector<std::string> stepIds;
        for (const auto& s : seqSteps) stepIds.push_back(s.stepId);

        // Assess feasibility
        auto feas = MigrationFeasibilityEngine::assess(pairId, sourceRuntime, targetRuntime,
                                                        riskScore, true, true);

        // Build candidate
        auto candidate = MigrationPathCandidateFactory::make(
            pairId, sourceRuntime, targetRuntime, stepIds, feas.score, feas.verdict);

        nlohmann::json stepsArr = nlohmann::json::array();
        for (const auto& s : seqSteps) stepsArr.push_back(MigrationStepSequencer::toJson(s));

        return {{"success", true}, {"pair_id", pairId},
                {"source_runtime", sourceRuntime}, {"target_runtime", targetRuntime},
                {"feasibility", feas.score}, {"verdict", feas.verdict},
                {"steps", stepsArr}, {"step_count", (int)seqSteps.size()}};
    }

    nlohmann::json runTrackMigrationProgress(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "pair_id required"}};
        std::string pairId = args.value("pair_id", "");
        std::string action = args.value("action", "");
        if (pairId.empty()) return {{"success", false}, {"error", "pair_id required"}};
        if (action.empty()) return {{"success", false}, {"error", "action required"}};

        if (action == "start") {
            int totalSteps = args.value("total_steps", 5);
            migrationTracker_.start(pairId, totalSteps);
            return {{"success", true}, {"pair_id", pairId}, {"action", "start"},
                    {"total_steps", totalSteps}};
        } else if (action == "advance") {
            migrationTracker_.advance(pairId);
        } else if (action == "complete") {
            migrationTracker_.complete(pairId);
        } else if (action == "fail") {
            migrationTracker_.fail(pairId);
        } else if (action != "get") {
            return {{"success", false}, {"error", "unknown action"}};
        }
        auto* pg = migrationTracker_.get(pairId);
        if (!pg) return {{"success", false}, {"error", "pair not tracked"}};
        auto j = MigrationProgressTracker::toJson(*pg);
        j["success"] = true;
        return j;
    }
