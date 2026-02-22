// Sprint 121 debugging workflow MCP tools (Steps 1456-1457)
// Included inside MCPServer class body.

    void registerDebugWorkflowTools() {
        tools_.push_back({"whetstone_capture_failure_packet",
            "Capture and normalize a deterministic failure packet from a command.",
            {{"type", "object"}, {"properties", {
                {"command", {{"type", "string"}}},
                {"cwd", {{"type", "string"}}},
                {"target", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"command"})}}
        });
        toolHandlers_["whetstone_capture_failure_packet"] =
            [this](const nlohmann::json& args) { return runCaptureFailurePacket(args); };

        tools_.push_back({"whetstone_debug_until_green",
            "Run deterministic debug loop until green or escalation.",
            {{"type", "object"}, {"properties", {
                {"command", {{"type", "string"}}},
                {"max_iterations", {{"type", "integer"}}},
                {"context_budget", {{"type", "string"}}},
                {"apply_patches", {{"type", "boolean"}}}
            }}, {"required", nlohmann::json::array({"command"})}}
        });
        toolHandlers_["whetstone_debug_until_green"] =
            [this](const nlohmann::json& args) { return runDebugUntilGreen(args); };

        tools_.push_back({"whetstone_cluster_failures",
            "Cluster failure packets into root-cause-first groups.",
            {{"type", "object"}, {"properties", {
                {"packets", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"packets"})}}
        });
        toolHandlers_["whetstone_cluster_failures"] =
            [this](const nlohmann::json& args) { return runClusterFailures(args); };

        tools_.push_back({"whetstone_assemble_fix_context",
            "Assemble minimal context slices for patch generation.",
            {{"type", "object"}, {"properties", {
                {"mode", {{"type", "string"}}},
                {"slices", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"slices"})}}
        });
        toolHandlers_["whetstone_assemble_fix_context"] =
            [this](const nlohmann::json& args) { return runAssembleFixContext(args); };

        tools_.push_back({"whetstone_propose_patch_for_failure",
            "Generate deterministic patch proposal from failure cluster and context.",
            {{"type", "object"}, {"properties", {
                {"cluster", {{"type", "object"}}},
                {"context", {{"type", "object"}}},
                {"test_target", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"cluster", "context"})}}
        });
        toolHandlers_["whetstone_propose_patch_for_failure"] =
            [this](const nlohmann::json& args) { return runProposePatchForFailure(args); };

        tools_.push_back({"whetstone_save_repro_packet",
            "Persist deterministic repro packet to disk.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}}},
                {"packet", {{"type", "object"}}}
            }}, {"required", nlohmann::json::array({"path", "packet"})}}
        });
        toolHandlers_["whetstone_save_repro_packet"] =
            [this](const nlohmann::json& args) { return runSaveReproPacket(args); };

        tools_.push_back({"whetstone_replay_repro_packet",
            "Replay and compare repro packet against a current failure packet.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}}},
                {"current_packet", {{"type", "object"}}}
            }}, {"required", nlohmann::json::array({"path", "current_packet"})}}
        });
        toolHandlers_["whetstone_replay_repro_packet"] =
            [this](const nlohmann::json& args) { return runReplayReproPacket(args); };

        tools_.push_back({"whetstone_regression_guard",
            "Plan deterministic regression guard set from touched files and step id.",
            {{"type", "object"}, {"properties", {
                {"touched_files", {{"type", "array"}}},
                {"step_id", {{"type", "integer"}}},
                {"failing_target", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"step_id"})}}
        });
        toolHandlers_["whetstone_regression_guard"] =
            [this](const nlohmann::json& args) { return runRegressionGuard(args); };

        tools_.push_back({"whetstone_export_repro_jsonl",
            "Export repro packets to JSONL for training/eval pipelines.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}}},
                {"packets", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"path", "packets"})}}
        });
        toolHandlers_["whetstone_export_repro_jsonl"] =
            [this](const nlohmann::json& args) { return runExportReproJsonl(args); };

        tools_.push_back({"whetstone_get_debug_metrics",
            "Compute deterministic debug metrics packet.",
            {{"type", "object"}, {"properties", {
                {"session_id", {{"type", "string"}}},
                {"iterations", {{"type", "integer"}}},
                {"duration_ms", {{"type", "integer"}}},
                {"token_cost", {{"type", "integer"}}},
                {"symptom_count", {{"type", "integer"}}},
                {"root_cause_count", {{"type", "integer"}}},
                {"accepted_patches", {{"type", "integer"}}},
                {"proposed_patches", {{"type", "integer"}}},
                {"regressions", {{"type", "integer"}}},
                {"total_runs", {{"type", "integer"}}}
            }}, {"required", nlohmann::json::array({"session_id"})}}
        });
        toolHandlers_["whetstone_get_debug_metrics"] =
            [this](const nlohmann::json& args) { return runGetDebugMetrics(args); };

        tools_.push_back({"whetstone_get_slm_debug_readiness",
            "Compute SLM debug readiness score from metrics packet.",
            {{"type", "object"}, {"properties", {
                {"metrics", {{"type", "object"}}}
            }}, {"required", nlohmann::json::array({"metrics"})}}
        });
        toolHandlers_["whetstone_get_slm_debug_readiness"] =
            [this](const nlohmann::json& args) { return runGetSlmDebugReadiness(args); };

        tools_.push_back({"whetstone_validate_patch_proposal",
            "Validate patch proposal invariants and scope.",
            {{"type", "object"}, {"properties", {
                {"proposal", {{"type", "object"}}},
                {"allowed_files", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"proposal", "allowed_files"})}}
        });
        toolHandlers_["whetstone_validate_patch_proposal"] =
            [this](const nlohmann::json& args) { return runValidatePatchProposal(args); };

        tools_.push_back({"whetstone_dry_run_patch",
            "Run deterministic dry-run validation on unified diff.",
            {{"type", "object"}, {"properties", {
                {"diff", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"diff"})}}
        });
        toolHandlers_["whetstone_dry_run_patch"] =
            [this](const nlohmann::json& args) { return runDryRunPatch(args); };

        tools_.push_back({"whetstone_apply_patch_packet",
            "Record deterministic patch apply event after validation and dry-run.",
            {{"type", "object"}, {"properties", {
                {"proposal_id", {{"type", "string"}}},
                {"diff", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"proposal_id", "diff"})}}
        });
        toolHandlers_["whetstone_apply_patch_packet"] =
            [this](const nlohmann::json& args) { return runApplyPatchPacket(args); };

        tools_.push_back({"whetstone_rollback_last_patch",
            "Rollback last patch execution record from ledger (logical rollback event).",
            {{"type", "object"}, {"properties", {
                {"ledger_path", {{"type", "string"}}}
            }}}
        });
        toolHandlers_["whetstone_rollback_last_patch"] =
            [this](const nlohmann::json& args) { return runRollbackLastPatch(args); };

        tools_.push_back({"whetstone_run_bisect_debug",
            "Select midpoint candidate from proposal set for bisect debugging.",
            {{"type", "object"}, {"properties", {
                {"proposal_ids", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"proposal_ids"})}}
        });
        toolHandlers_["whetstone_run_bisect_debug"] =
            [this](const nlohmann::json& args) { return runBisectDebug(args); };

        tools_.push_back({"whetstone_start_debug_campaign",
            "Create deterministic debug campaign and initialize progress.",
            {{"type", "object"}, {"properties", {
                {"name", {{"type", "string"}}},
                {"targets", {{"type", "array"}}},
                {"budget_mode", {{"type", "string"}}},
                {"max_iterations_per_target", {{"type", "integer"}}},
                {"apply_patches", {{"type", "boolean"}}}
            }}, {"required", nlohmann::json::array({"name", "targets"})}}
        });
        toolHandlers_["whetstone_start_debug_campaign"] =
            [this](const nlohmann::json& args) { return runStartDebugCampaign(args); };

        tools_.push_back({"whetstone_step_debug_campaign",
            "Advance one target in debug campaign respecting safety envelope.",
            {{"type", "object"}, {"properties", {
                {"campaign_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"campaign_id"})}}
        });
        toolHandlers_["whetstone_step_debug_campaign"] =
            [this](const nlohmann::json& args) { return runStepDebugCampaign(args); };

        tools_.push_back({"whetstone_get_debug_campaign_status",
            "Return current campaign progress snapshot.",
            {{"type", "object"}, {"properties", {
                {"campaign_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"campaign_id"})}}
        });
        toolHandlers_["whetstone_get_debug_campaign_status"] =
            [this](const nlohmann::json& args) { return runGetDebugCampaignStatus(args); };

        tools_.push_back({"whetstone_stop_debug_campaign",
            "Stop campaign and mark progress as stopped.",
            {{"type", "object"}, {"properties", {
                {"campaign_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"campaign_id"})}}
        });
        toolHandlers_["whetstone_stop_debug_campaign"] =
            [this](const nlohmann::json& args) { return runStopDebugCampaign(args); };

        tools_.push_back({"whetstone_save_campaign_checkpoint",
            "Save a deterministic checkpoint snapshot for a campaign.",
            {{"type", "object"}, {"properties", {
                {"campaign_id", {{"type", "string"}}},
                {"sequence", {{"type", "integer"}}},
                {"notes", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"campaign_id", "sequence"})}}
        });
        toolHandlers_["whetstone_save_campaign_checkpoint"] =
            [this](const nlohmann::json& args) { return runSaveCampaignCheckpoint(args); };

        tools_.push_back({"whetstone_list_campaign_checkpoints",
            "List deterministic checkpoint snapshots for a campaign.",
            {{"type", "object"}, {"properties", {
                {"campaign_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"campaign_id"})}}
        });
        toolHandlers_["whetstone_list_campaign_checkpoints"] =
            [this](const nlohmann::json& args) { return runListCampaignCheckpoints(args); };

        tools_.push_back({"whetstone_resume_debug_campaign",
            "Resume campaign progress from latest deterministic checkpoint.",
            {{"type", "object"}, {"properties", {
                {"campaign_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"campaign_id"})}}
        });
        toolHandlers_["whetstone_resume_debug_campaign"] =
            [this](const nlohmann::json& args) { return runResumeDebugCampaign(args); };

        tools_.push_back({"whetstone_export_debug_campaign_bundle",
            "Export campaign spec/progress/checkpoints as machine-readable bundle.",
            {{"type", "object"}, {"properties", {
                {"campaign_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"campaign_id"})}}
        });
        toolHandlers_["whetstone_export_debug_campaign_bundle"] =
            [this](const nlohmann::json& args) { return runExportDebugCampaignBundle(args); };

        tools_.push_back({"whetstone_generate_debug_hints",
            "Generate deterministic debug hint packet from failure class and context.",
            {{"type", "object"}, {"properties", {
                {"failure_class", {{"type", "string"}}},
                {"context", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"failure_class"})}}
        });
        toolHandlers_["whetstone_generate_debug_hints"] =
            [this](const nlohmann::json& args) { return runGenerateDebugHints(args); };

        tools_.push_back({"whetstone_score_failure_triage",
            "Compute deterministic triage score for a failure.",
            {{"type", "object"}, {"properties", {
                {"failure_class", {{"type", "string"}}},
                {"severity", {{"type", "integer"}}},
                {"reproducibility", {{"type", "integer"}}},
                {"blast_radius", {{"type", "integer"}}}
            }}, {"required", nlohmann::json::array({"failure_class"})}}
        });
        toolHandlers_["whetstone_score_failure_triage"] =
            [this](const nlohmann::json& args) { return runScoreFailureTriage(args); };

        tools_.push_back({"whetstone_reduce_repro_command",
            "Deterministically remove optional flags from repro command.",
            {{"type", "object"}, {"properties", {
                {"command", {{"type", "string"}}},
                {"removable_flags", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"command"})}}
        });
        toolHandlers_["whetstone_reduce_repro_command"] =
            [this](const nlohmann::json& args) { return runReduceReproCommand(args); };

        tools_.push_back({"whetstone_label_patch_risk",
            "Label patch risk level with deterministic rationale.",
            {{"type", "object"}, {"properties", {
                {"files_touched", {{"type", "integer"}}},
                {"line_changes", {{"type", "integer"}}},
                {"core_path_touched", {{"type", "boolean"}}},
                {"adds_unsafe_pattern", {{"type", "boolean"}}}
            }}}
        });
        toolHandlers_["whetstone_label_patch_risk"] =
            [this](const nlohmann::json& args) { return runLabelPatchRisk(args); };

        tools_.push_back({"whetstone_get_debug_recipe",
            "Get deterministic debug recipe by failure class.",
            {{"type", "object"}, {"properties", {
                {"failure_class", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"failure_class"})}}
        });
        toolHandlers_["whetstone_get_debug_recipe"] =
            [this](const nlohmann::json& args) { return runGetDebugRecipe(args); };

        tools_.push_back({"whetstone_validate_debug_action",
            "Validate proposed debug action against deterministic rules.",
            {{"type", "object"}, {"properties", {
                {"action", {{"type", "string"}}},
                {"touches_forbidden_path", {{"type", "boolean"}}},
                {"changes_many_files", {{"type", "boolean"}}},
                {"bypasses_tests", {{"type", "boolean"}}}
            }}, {"required", nlohmann::json::array({"action"})}}
        });
        toolHandlers_["whetstone_validate_debug_action"] =
            [this](const nlohmann::json& args) { return runValidateDebugAction(args); };

        tools_.push_back({"whetstone_record_debug_trace",
            "Record deterministic debug trace event by trace id.",
            {{"type", "object"}, {"properties", {
                {"trace_id", {{"type", "string"}}},
                {"index", {{"type", "integer"}}},
                {"action", {{"type", "string"}}},
                {"status", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"trace_id", "index", "action", "status"})}}
        });
        toolHandlers_["whetstone_record_debug_trace"] =
            [this](const nlohmann::json& args) { return runRecordDebugTrace(args); };

        tools_.push_back({"whetstone_get_debug_trace",
            "Read deterministic debug trace events by trace id.",
            {{"type", "object"}, {"properties", {
                {"trace_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"trace_id"})}}
        });
        toolHandlers_["whetstone_get_debug_trace"] =
            [this](const nlohmann::json& args) { return runGetDebugTrace(args); };

        tools_.push_back({"whetstone_get_debug_constraints",
            "Get deterministic debugging policy constraints by mode.",
            {{"type", "object"}, {"properties", {
                {"mode", {{"type", "string"}}}
            }}}
        });
        toolHandlers_["whetstone_get_debug_constraints"] =
            [this](const nlohmann::json& args) { return runGetDebugConstraints(args); };

        tools_.push_back({"whetstone_estimate_debug_budget",
            "Estimate deterministic debug budget from profile and workload.",
            {{"type", "object"}, {"properties", {
                {"profile", {{"type", "string"}}},
                {"failing_targets", {{"type", "integer"}}},
                {"complexity", {{"type", "integer"}}}
            }}}
        });
        toolHandlers_["whetstone_estimate_debug_budget"] =
            [this](const nlohmann::json& args) { return runEstimateDebugBudget(args); };

        tools_.push_back({"whetstone_get_recovery_advice",
            "Get deterministic recovery advice for a debug stop reason.",
            {{"type", "object"}, {"properties", {
                {"stop_reason", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"stop_reason"})}}
        });
        toolHandlers_["whetstone_get_recovery_advice"] =
            [this](const nlohmann::json& args) { return runGetRecoveryAdvice(args); };

        tools_.push_back({"whetstone_classify_debug_stop_reason",
            "Classify deterministic debug stop reason from signals.",
            {{"type", "object"}, {"properties", {
                {"budget_exceeded", {{"type", "boolean"}}},
                {"policy_violation", {{"type", "boolean"}}},
                {"tests_failing", {{"type", "boolean"}}},
                {"patch_rejected", {{"type", "boolean"}}}
            }}}
        });
        toolHandlers_["whetstone_classify_debug_stop_reason"] =
            [this](const nlohmann::json& args) { return runClassifyDebugStopReason(args); };

        tools_.push_back({"whetstone_get_debug_checklist",
            "Get deterministic debug checklist template for a session/failure class.",
            {{"type", "object"}, {"properties", {
                {"session_id", {{"type", "string"}}},
                {"failure_class", {{"type", "string"}}}
            }}}
        });
        toolHandlers_["whetstone_get_debug_checklist"] =
            [this](const nlohmann::json& args) { return runGetDebugChecklist(args); };

        tools_.push_back({"whetstone_mark_debug_checklist_item",
            "Mark a debug checklist item done/undone for a session.",
            {{"type", "object"}, {"properties", {
                {"session_id", {{"type", "string"}}},
                {"index", {{"type", "integer"}}},
                {"done", {{"type", "boolean"}}}
            }}, {"required", nlohmann::json::array({"session_id", "index"})}}
        });
        toolHandlers_["whetstone_mark_debug_checklist_item"] =
            [this](const nlohmann::json& args) { return runMarkDebugChecklistItem(args); };

        tools_.push_back({"whetstone_build_debug_handoff",
            "Build deterministic session handoff packet with next actions.",
            {{"type", "object"}, {"properties", {
                {"session_id", {{"type", "string"}}},
                {"summary", {{"type", "string"}}},
                {"next_actions", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"session_id"})}}
        });
        toolHandlers_["whetstone_build_debug_handoff"] =
            [this](const nlohmann::json& args) { return runBuildDebugHandoff(args); };

        tools_.push_back({"whetstone_get_debug_evidence_index",
            "Get deterministic evidence index for a debug session.",
            {{"type", "object"}, {"properties", {
                {"session_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"session_id"})}}
        });
        toolHandlers_["whetstone_get_debug_evidence_index"] =
            [this](const nlohmann::json& args) { return runGetDebugEvidenceIndex(args); };
    }

    nlohmann::json runCaptureFailurePacket(const nlohmann::json& args) {
        if (!args.contains("command") || !args["command"].is_string()) {
            return {{"success", false}, {"error", "command_missing"}};
        }
        const std::string command = args.value("command", "");
        std::string raw = "runtime failure";
        int exitCode = 1;
        if (command.rfind("mock:", 0) == 0) {
            raw = command.substr(5);
            exitCode = raw.find("ok") != std::string::npos ? 0 : 1;
        }
        auto packet = FailurePacketModel::make(command, raw, exitCode, 8192);
        return {{"success", true}, {"packet", FailurePacketModel::toJson(packet)}};
    }

    nlohmann::json runDebugUntilGreen(const nlohmann::json& args) {
        if (!args.contains("command") || !args["command"].is_string()) {
            return {{"success", false}, {"error", "command_missing"}};
        }
        const std::string command = args.value("command", "");
        const int maxIter = args.value("max_iterations", 3);
        const std::string budget = args.value("context_budget", "tiny");
        const bool apply = args.value("apply_patches", true);

        auto r = DebugLoopOrchestrator::run(command, maxIter, budget, apply);

        return {
            {"success", true},
            {"status", r.status},
            {"iterations", r.iterations},
            {"state_transitions", r.transitions},
            {"final_packet", FailurePacketModel::toJson(r.finalPacket)},
            {"final_guard_report", RegressionGuardPlanner::toJson(r.guard)}
        };
    }

    nlohmann::json runClusterFailures(const nlohmann::json& args) {
        if (!args.contains("packets") || !args["packets"].is_array()) {
            return {{"success", false}, {"error", "packets_missing"}};
        }
        std::vector<FailurePacket> packets;
        for (const auto& j : args["packets"]) packets.push_back(FailurePacketModel::fromJson(j));
        auto clusters = FailureClusterer::cluster(packets);
        return {{"success", true}, {"clusters", FailureClusterer::toJson(clusters)}};
    }

    nlohmann::json runAssembleFixContext(const nlohmann::json& args) {
        if (!args.contains("slices") || !args["slices"].is_array()) {
            return {{"success", false}, {"error", "slices_missing"}};
        }
        std::string mode = args.value("mode", "tiny");
        std::vector<FixContextSlice> slices;
        for (const auto& s : args["slices"]) {
            slices.push_back({
                s.value("path", ""),
                s.value("start_line", 1),
                s.value("end_line", 1),
                s.value("text", "")
            });
        }
        auto packet = FixContextAssembler::assemble(mode, slices);
        return {{"success", true}, {"context", FixContextAssembler::toJson(packet)}};
    }

    nlohmann::json runProposePatchForFailure(const nlohmann::json& args) {
        if (!args.contains("cluster") || !args["cluster"].is_object()) {
            return {{"success", false}, {"error", "cluster_missing"}};
        }
        if (!args.contains("context") || !args["context"].is_object()) {
            return {{"success", false}, {"error", "context_missing"}};
        }
        FailureCluster c;
        c.clusterId = args["cluster"].value("cluster_id", "");
        c.failureClass = args["cluster"].value("failure_class", "unknown");
        c.fixFirst = args["cluster"].value("fix_first", false);

        FixContextPacket ctx;
        ctx.mode = args["context"].value("mode", "tiny");
        for (const auto& s : args["context"].value("slices", nlohmann::json::array())) {
            ctx.slices.push_back({
                s.value("path", ""),
                s.value("start_line", 1),
                s.value("end_line", 1),
                s.value("text", "")
            });
        }
        std::string testTarget = args.value("test_target", "");
        auto suggestion = FailurePatchProposer::propose(c, ctx, testTarget);
        return {
            {"success", true},
            {"confidence", suggestion.confidence},
            {"rationale", suggestion.rationale},
            {"proposal", PatchProposalModel::toJson(suggestion.proposal)}
        };
    }

    nlohmann::json runSaveReproPacket(const nlohmann::json& args) {
        if (!args.contains("path") || !args["path"].is_string()) {
            return {{"success", false}, {"error", "path_missing"}};
        }
        if (!args.contains("packet") || !args["packet"].is_object()) {
            return {{"success", false}, {"error", "packet_missing"}};
        }
        ReproPacket rp;
        const auto& j = args["packet"];
        rp.failure = FailurePacketModel::fromJson(j.value("failure", nlohmann::json::object()));
        rp.envHash = j.value("env_hash", "");
        rp.fileHashes = j.value("file_hashes", nlohmann::json::object());
        rp.patchHistory = j.value("patch_history", nlohmann::json::array());
        rp.reproCommand = j.value("repro_command", "");

        bool ok = ReproPacketStore::save(args.value("path", ""), rp);
        return {{"success", ok}, {"path", args.value("path", "")}};
    }

    nlohmann::json runReplayReproPacket(const nlohmann::json& args) {
        if (!args.contains("path") || !args["path"].is_string()) {
            return {{"success", false}, {"error", "path_missing"}};
        }
        if (!args.contains("current_packet") || !args["current_packet"].is_object()) {
            return {{"success", false}, {"error", "current_packet_missing"}};
        }
        ReproPacket rp;
        if (!ReproPacketStore::load(args.value("path", ""), &rp)) {
            return {{"success", false}, {"error", "repro_load_failed"}};
        }
        auto current = FailurePacketModel::fromJson(args["current_packet"]);
        bool match = ReproPacketStore::replayClassMatches(rp, current);
        return {{"success", true}, {"match", match}, {"archived_failure_class", rp.failure.failureClass}};
    }

    nlohmann::json runRegressionGuard(const nlohmann::json& args) {
        int stepId = args.value("step_id", 0);
        if (stepId <= 0) return {{"success", false}, {"error", "step_id_invalid"}};
        std::vector<std::string> touched;
        for (const auto& f : args.value("touched_files", nlohmann::json::array())) {
            if (f.is_string()) touched.push_back(f.get<std::string>());
        }
        auto plan = RegressionGuardPlanner::plan(touched, stepId, args.value("failing_target", ""));
        return {{"success", true}, {"guard", RegressionGuardPlanner::toJson(plan)}};
    }

    nlohmann::json runExportReproJsonl(const nlohmann::json& args) {
        if (!args.contains("path") || !args["path"].is_string()) return {{"success", false}, {"error", "path_missing"}};
        if (!args.contains("packets") || !args["packets"].is_array()) return {{"success", false}, {"error", "packets_missing"}};
        std::vector<ReproPacket> packets;
        for (const auto& p : args["packets"]) {
            packets.push_back(ReproPacketStore::fromJson(p));
        }
        bool ok = ReproPacketStore::exportJsonl(args.value("path", ""), packets);
        return {{"success", ok}, {"path", args.value("path", "")}, {"count", (int)packets.size()}};
    }

    nlohmann::json runGetDebugMetrics(const nlohmann::json& args) {
        if (!args.contains("session_id") || !args["session_id"].is_string()) {
            return {{"success", false}, {"error", "session_id_missing"}};
        }
        auto m = DebugMetricsModel::compute(
            args.value("session_id", ""),
            args.value("iterations", 0),
            args.value("duration_ms", 0),
            args.value("token_cost", 0),
            args.value("symptom_count", 0),
            args.value("root_cause_count", 0),
            args.value("accepted_patches", 0),
            args.value("proposed_patches", 0),
            args.value("regressions", 0),
            args.value("total_runs", 0));
        return {{"success", true}, {"metrics", DebugMetricsModel::toJson(m)}};
    }

    nlohmann::json runGetSlmDebugReadiness(const nlohmann::json& args) {
        if (!args.contains("metrics") || !args["metrics"].is_object()) {
            return {{"success", false}, {"error", "metrics_missing"}};
        }
        auto s = SLMDebugReadinessModel::score(args["metrics"]);
        return {{"success", true}, {"readiness", SLMDebugReadinessModel::toJson(s)}};
    }

    nlohmann::json runValidatePatchProposal(const nlohmann::json& args) {
        if (!args.contains("proposal") || !args["proposal"].is_object()) return {{"success", false}, {"error", "proposal_missing"}};
        if (!args.contains("allowed_files") || !args["allowed_files"].is_array()) return {{"success", false}, {"error", "allowed_files_missing"}};
        auto p = PatchProposalModel::fromJson(args["proposal"]);
        std::vector<std::string> allowed;
        for (const auto& f : args["allowed_files"]) if (f.is_string()) allowed.push_back(f.get<std::string>());
        auto check = PatchInvariantChecker::check(p, allowed);
        return {{"success", true}, {"result", PatchInvariantChecker::toJson(check)}};
    }

    nlohmann::json runDryRunPatch(const nlohmann::json& args) {
        if (!args.contains("diff") || !args["diff"].is_string()) return {{"success", false}, {"error", "diff_missing"}};
        auto r = PatchDryRunApplier::run(args.value("diff", ""));
        return {{"success", true}, {"result", PatchDryRunApplier::toJson(r)}};
    }

    nlohmann::json runApplyPatchPacket(const nlohmann::json& args) {
        if (!args.contains("proposal_id") || !args["proposal_id"].is_string()) return {{"success", false}, {"error", "proposal_id_missing"}};
        if (!args.contains("diff") || !args["diff"].is_string()) return {{"success", false}, {"error", "diff_missing"}};
        auto dry = PatchDryRunApplier::run(args.value("diff", ""));
        auto rec = PatchExecutionRecordModel::make(args.value("proposal_id", ""), "applied", dry.success,
                                                   dry.success ? "applied_logical" : "dry_run_failed");
        (void)RollbackLedger::append("/tmp/whetstone_patch_ledger.json", rec);
        return {{"success", dry.success}, {"record", PatchExecutionRecordModel::toJson(rec)}, {"dry_run", PatchDryRunApplier::toJson(dry)}};
    }

    nlohmann::json runRollbackLastPatch(const nlohmann::json& args) {
        std::string path = args.value("ledger_path", "/tmp/whetstone_patch_ledger.json");
        auto last = RollbackLedger::last(path);
        if (last.is_null() || last.empty()) return {{"success", false}, {"error", "ledger_empty"}};
        auto rec = PatchExecutionRecordModel::make(last.value("proposal_id", ""), "rolled_back", true, "logical_rollback");
        (void)RollbackLedger::append(path, rec);
        return {{"success", true}, {"record", PatchExecutionRecordModel::toJson(rec)}};
    }

    nlohmann::json runBisectDebug(const nlohmann::json& args) {
        if (!args.contains("proposal_ids") || !args["proposal_ids"].is_array()) return {{"success", false}, {"error", "proposal_ids_missing"}};
        std::vector<std::string> ids;
        for (const auto& x : args["proposal_ids"]) if (x.is_string()) ids.push_back(x.get<std::string>());
        auto ordered = BisectCandidateSelector::ordered(ids);
        auto mid = BisectCandidateSelector::midpoint(ordered);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& c : ordered) arr.push_back({{"proposal_id", c.proposalId}, {"index", c.index}});
        return {{"success", true}, {"ordered_candidates", arr}, {"midpoint", {{"proposal_id", mid.proposalId}, {"index", mid.index}}}};
    }

    nlohmann::json runStartDebugCampaign(const nlohmann::json& args) {
        if (!args.contains("name") || !args["name"].is_string()) return {{"success", false}, {"error", "name_missing"}};
        if (!args.contains("targets") || !args["targets"].is_array()) return {{"success", false}, {"error", "targets_missing"}};
        std::vector<std::string> targets;
        for (const auto& t : args["targets"]) if (t.is_string()) targets.push_back(t.get<std::string>());
        auto spec = DebugCampaignSpecModel::make(
            args.value("name", ""),
            targets,
            args.value("budget_mode", "tiny"),
            args.value("max_iterations_per_target", 3),
            args.value("apply_patches", true));
        CampaignProgress prog;
        prog.campaignId = spec.campaignId;
        prog.totalTargets = static_cast<int>(spec.targets.size());
        campaignSpecs()[spec.campaignId] = spec;
        campaignProgress()[spec.campaignId] = prog;
        return {{"success", true}, {"campaign", DebugCampaignSpecModel::toJson(spec)}, {"progress", CampaignProgressTracker::toJson(prog)}};
    }

    nlohmann::json runStepDebugCampaign(const nlohmann::json& args) {
        if (!args.contains("campaign_id") || !args["campaign_id"].is_string()) return {{"success", false}, {"error", "campaign_id_missing"}};
        const std::string id = args.value("campaign_id", "");
        auto it = campaignProgress().find(id);
        if (it == campaignProgress().end()) return {{"success", false}, {"error", "campaign_not_found"}};
        auto& p = it->second;
        if (p.stopped) return {{"success", false}, {"error", "campaign_stopped"}};
        if (p.completedTargets < p.totalTargets) {
            ++p.completedTargets;
            ++p.greenTargets;
        }
        auto safety = SafetyEnvelopePolicy::evaluate(SafetyEnvelope{}, p.escalatedTargets, 0, p.completedTargets * 1000);
        if (!safety.allowed) p.stopped = true;
        return {{"success", true}, {"progress", CampaignProgressTracker::toJson(p)}, {"safety", SafetyEnvelopePolicy::toJson(safety)}};
    }

    nlohmann::json runGetDebugCampaignStatus(const nlohmann::json& args) {
        if (!args.contains("campaign_id") || !args["campaign_id"].is_string()) return {{"success", false}, {"error", "campaign_id_missing"}};
        const std::string id = args.value("campaign_id", "");
        auto it = campaignProgress().find(id);
        if (it == campaignProgress().end()) return {{"success", false}, {"error", "campaign_not_found"}};
        return {{"success", true}, {"progress", CampaignProgressTracker::toJson(it->second)}};
    }

    nlohmann::json runStopDebugCampaign(const nlohmann::json& args) {
        if (!args.contains("campaign_id") || !args["campaign_id"].is_string()) return {{"success", false}, {"error", "campaign_id_missing"}};
        const std::string id = args.value("campaign_id", "");
        auto it = campaignProgress().find(id);
        if (it == campaignProgress().end()) return {{"success", false}, {"error", "campaign_not_found"}};
        it->second.stopped = true;
        return {{"success", true}, {"progress", CampaignProgressTracker::toJson(it->second)}};
    }

    nlohmann::json runSaveCampaignCheckpoint(const nlohmann::json& args) {
        if (!args.contains("campaign_id") || !args["campaign_id"].is_string()) return {{"success", false}, {"error", "campaign_id_missing"}};
        if (!args.contains("sequence") || !args["sequence"].is_number_integer()) return {{"success", false}, {"error", "sequence_missing"}};
        const std::string id = args.value("campaign_id", "");
        auto it = campaignProgress().find(id);
        if (it == campaignProgress().end()) return {{"success", false}, {"error", "campaign_not_found"}};
        auto cp = CampaignCheckpointModel::make(id, args.value("sequence", 0), it->second, args.value("notes", ""));
        checkpointsByCampaign()[id].push_back(cp);
        std::sort(checkpointsByCampaign()[id].begin(), checkpointsByCampaign()[id].end(),
                  [](const CampaignCheckpoint& a, const CampaignCheckpoint& b) {
                      if (a.sequence != b.sequence) return a.sequence < b.sequence;
                      return a.checkpointId < b.checkpointId;
                  });
        return {{"success", true}, {"checkpoint", CampaignCheckpointModel::toJson(cp)}};
    }

    nlohmann::json runListCampaignCheckpoints(const nlohmann::json& args) {
        if (!args.contains("campaign_id") || !args["campaign_id"].is_string()) return {{"success", false}, {"error", "campaign_id_missing"}};
        const std::string id = args.value("campaign_id", "");
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& c : checkpointsByCampaign()[id]) arr.push_back(CampaignCheckpointModel::toJson(c));
        return {{"success", true}, {"campaign_id", id}, {"checkpoints", arr}};
    }

    nlohmann::json runResumeDebugCampaign(const nlohmann::json& args) {
        if (!args.contains("campaign_id") || !args["campaign_id"].is_string()) return {{"success", false}, {"error", "campaign_id_missing"}};
        const std::string id = args.value("campaign_id", "");
        auto pIt = campaignProgress().find(id);
        if (pIt == campaignProgress().end()) return {{"success", false}, {"error", "campaign_not_found"}};
        auto plan = CampaignResumePlanner::plan(id, checkpointsByCampaign()[id]);
        if (!plan.canResume) return {{"success", false}, {"error", "checkpoint_not_found"}};
        pIt->second = plan.restoredProgress;
        pIt->second.campaignId = id;
        return {{"success", true}, {"plan", CampaignResumePlanner::toJson(plan)}, {"progress", CampaignProgressTracker::toJson(pIt->second)}};
    }

    nlohmann::json runExportDebugCampaignBundle(const nlohmann::json& args) {
        if (!args.contains("campaign_id") || !args["campaign_id"].is_string()) return {{"success", false}, {"error", "campaign_id_missing"}};
        const std::string id = args.value("campaign_id", "");
        auto sIt = campaignSpecs().find(id);
        auto pIt = campaignProgress().find(id);
        if (sIt == campaignSpecs().end() || pIt == campaignProgress().end()) {
            return {{"success", false}, {"error", "campaign_not_found"}};
        }
        nlohmann::json metrics = {
            {"checkpoint_count", (int)checkpointsByCampaign()[id].size()},
            {"progress_percent", CampaignProgressTracker::percent(pIt->second)}
        };
        auto bundle = CampaignExportBundleModel::build(sIt->second, pIt->second, checkpointsByCampaign()[id], metrics);
        return {{"success", true}, {"bundle", CampaignExportBundleModel::toJson(bundle)}};
    }

    nlohmann::json runGenerateDebugHints(const nlohmann::json& args) {
        if (!args.contains("failure_class") || !args["failure_class"].is_string()) {
            return {{"success", false}, {"error", "failure_class_missing"}};
        }
        std::vector<std::string> context;
        for (const auto& c : args.value("context", nlohmann::json::array())) {
            if (c.is_string()) context.push_back(c.get<std::string>());
        }
        auto hints = DebugHintTemplate::build(args.value("failure_class", ""), context);
        return {{"success", true}, {"hints", DebugHintTemplate::toJson(hints)}};
    }

    nlohmann::json runScoreFailureTriage(const nlohmann::json& args) {
        if (!args.contains("failure_class") || !args["failure_class"].is_string()) {
            return {{"success", false}, {"error", "failure_class_missing"}};
        }
        auto triage = FailureTriageScorer::compute(
            args.value("failure_class", ""),
            args.value("severity", 0),
            args.value("reproducibility", 0),
            args.value("blast_radius", 0));
        return {{"success", true}, {"triage", FailureTriageScorer::toJson(triage)}};
    }

    nlohmann::json runReduceReproCommand(const nlohmann::json& args) {
        if (!args.contains("command") || !args["command"].is_string()) {
            return {{"success", false}, {"error", "command_missing"}};
        }
        std::vector<std::string> flags;
        for (const auto& f : args.value("removable_flags", nlohmann::json::array())) {
            if (f.is_string()) flags.push_back(f.get<std::string>());
        }
        auto r = MinimalReproReducer::reduce(args.value("command", ""), flags);
        return {{"success", true}, {"repro", MinimalReproReducer::toJson(r)}};
    }

    nlohmann::json runLabelPatchRisk(const nlohmann::json& args) {
        auto risk = PatchRiskLabeler::label(
            args.value("files_touched", 0),
            args.value("line_changes", 0),
            args.value("core_path_touched", false),
            args.value("adds_unsafe_pattern", false));
        return {{"success", true}, {"risk", PatchRiskLabeler::toJson(risk)}};
    }

    nlohmann::json runGetDebugRecipe(const nlohmann::json& args) {
        if (!args.contains("failure_class") || !args["failure_class"].is_string()) {
            return {{"success", false}, {"error", "failure_class_missing"}};
        }
        auto recipe = DebugRecipeLibrary::get(args.value("failure_class", ""));
        return {{"success", true}, {"recipe", DebugRecipeLibrary::toJson(recipe)}};
    }

    nlohmann::json runValidateDebugAction(const nlohmann::json& args) {
        if (!args.contains("action") || !args["action"].is_string()) {
            return {{"success", false}, {"error", "action_missing"}};
        }
        auto v = DebugActionValidator::validate(
            args.value("action", ""),
            args.value("touches_forbidden_path", false),
            args.value("changes_many_files", false),
            args.value("bypasses_tests", false));
        return {{"success", true}, {"validation", DebugActionValidator::toJson(v)}};
    }

    nlohmann::json runRecordDebugTrace(const nlohmann::json& args) {
        if (!args.contains("trace_id") || !args["trace_id"].is_string()) return {{"success", false}, {"error", "trace_id_missing"}};
        if (!args.contains("index") || !args["index"].is_number_integer()) return {{"success", false}, {"error", "index_missing"}};
        if (!args.contains("action") || !args["action"].is_string()) return {{"success", false}, {"error", "action_missing"}};
        if (!args.contains("status") || !args["status"].is_string()) return {{"success", false}, {"error", "status_missing"}};
        const std::string traceId = args.value("trace_id", "");
        auto events = DebugExecutionTrace::append(
            traceEventsById()[traceId],
            {args.value("index", 0), args.value("action", ""), args.value("status", "")});
        traceEventsById()[traceId] = events;
        return {{"success", true}, {"trace_id", traceId}, {"trace", DebugExecutionTrace::toJson(events)}};
    }

    nlohmann::json runGetDebugTrace(const nlohmann::json& args) {
        if (!args.contains("trace_id") || !args["trace_id"].is_string()) return {{"success", false}, {"error", "trace_id_missing"}};
        const std::string traceId = args.value("trace_id", "");
        return {{"success", true}, {"trace_id", traceId}, {"trace", DebugExecutionTrace::toJson(traceEventsById()[traceId])}};
    }

    nlohmann::json runGetDebugConstraints(const nlohmann::json& args) {
        auto c = DebugPolicyConstraintModel::forMode(args.value("mode", "safe"));
        return {{"success", true}, {"constraints", DebugPolicyConstraintModel::toJson(c)}};
    }

    nlohmann::json runEstimateDebugBudget(const nlohmann::json& args) {
        auto b = DebugActionBudgetModel::estimate(
            args.value("profile", "tiny"),
            args.value("failing_targets", 1),
            args.value("complexity", 1));
        return {{"success", true}, {"budget", DebugActionBudgetModel::toJson(b)}};
    }

    nlohmann::json runGetRecoveryAdvice(const nlohmann::json& args) {
        if (!args.contains("stop_reason") || !args["stop_reason"].is_string()) {
            return {{"success", false}, {"error", "stop_reason_missing"}};
        }
        auto r = DebugRecoveryAdvisor::advise(args.value("stop_reason", ""));
        return {{"success", true}, {"recovery", DebugRecoveryAdvisor::toJson(r)}};
    }

    nlohmann::json runClassifyDebugStopReason(const nlohmann::json& args) {
        auto r = DebugStopReasonClassifier::classify(
            args.value("budget_exceeded", false),
            args.value("policy_violation", false),
            args.value("tests_failing", false),
            args.value("patch_rejected", false));
        return {{"success", true}, {"stop_reason", DebugStopReasonClassifier::toJson(r)}};
    }

    nlohmann::json runGetDebugChecklist(const nlohmann::json& args) {
        const std::string sessionId = args.value("session_id", "default");
        const std::string failureClass = args.value("failure_class", "generic");
        auto it = checklistBySession().find(sessionId);
        if (it == checklistBySession().end()) {
            checklistBySession()[sessionId] = DebugChecklistTemplateModel::build(failureClass);
        }
        return {{"success", true}, {"session_id", sessionId}, {"checklist", DebugChecklistTemplateModel::toJson(checklistBySession()[sessionId])}};
    }

    nlohmann::json runMarkDebugChecklistItem(const nlohmann::json& args) {
        if (!args.contains("session_id") || !args["session_id"].is_string()) return {{"success", false}, {"error", "session_id_missing"}};
        if (!args.contains("index") || !args["index"].is_number_integer()) return {{"success", false}, {"error", "index_missing"}};
        const std::string sessionId = args.value("session_id", "");
        int index = args.value("index", 0);
        bool done = args.value("done", true);
        auto it = checklistBySession().find(sessionId);
        if (it == checklistBySession().end()) checklistBySession()[sessionId] = DebugChecklistTemplateModel::build("generic");
        auto& items = checklistBySession()[sessionId].items;
        bool found = false;
        for (auto& i : items) {
            if (i.index == index) { i.done = done; found = true; break; }
        }
        if (!found) return {{"success", false}, {"error", "index_not_found"}};
        return {{"success", true}, {"session_id", sessionId}, {"checklist", DebugChecklistTemplateModel::toJson(checklistBySession()[sessionId])}};
    }

    nlohmann::json runBuildDebugHandoff(const nlohmann::json& args) {
        if (!args.contains("session_id") || !args["session_id"].is_string()) return {{"success", false}, {"error", "session_id_missing"}};
        const std::string sessionId = args.value("session_id", "");
        std::vector<std::string> nextActions;
        for (const auto& a : args.value("next_actions", nlohmann::json::array())) {
            if (a.is_string()) nextActions.push_back(a.get<std::string>());
        }
        if (nextActions.empty()) nextActions = {"Replay repro command", "Apply smallest safe patch", "Run regression guard"};
        auto handoff = DebugSessionHandoffModel::build(sessionId, args.value("summary", "Deterministic handoff"), nextActions);
        handoffBySession()[sessionId] = handoff;
        return {{"success", true}, {"handoff", DebugSessionHandoffModel::toJson(handoff)}};
    }

    nlohmann::json runGetDebugEvidenceIndex(const nlohmann::json& args) {
        if (!args.contains("session_id") || !args["session_id"].is_string()) return {{"success", false}, {"error", "session_id_missing"}};
        const std::string sessionId = args.value("session_id", "");
        std::vector<DebugEvidenceEntry> out;
        auto cIt = checklistBySession().find(sessionId);
        if (cIt != checklistBySession().end()) {
            for (const auto& i : cIt->second.items) {
                out.push_back({"checklist_item", sessionId + ":" + std::to_string(i.index) + ":" + (i.done ? "done" : "todo")});
            }
        }
        auto hIt = handoffBySession().find(sessionId);
        if (hIt != handoffBySession().end()) out.push_back({"handoff", sessionId});
        out = DebugEvidenceIndexModel::sort(out);
        return {{"success", true}, {"session_id", sessionId}, {"evidence", DebugEvidenceIndexModel::toJson(out)}};
    }

    static std::map<std::string, DebugCampaignSpec>& campaignSpecs() {
        static std::map<std::string, DebugCampaignSpec> s;
        return s;
    }

    static std::map<std::string, CampaignProgress>& campaignProgress() {
        static std::map<std::string, CampaignProgress> s;
        return s;
    }

    static std::map<std::string, std::vector<CampaignCheckpoint>>& checkpointsByCampaign() {
        static std::map<std::string, std::vector<CampaignCheckpoint>> s;
        return s;
    }

    static std::map<std::string, std::vector<DebugTraceEvent>>& traceEventsById() {
        static std::map<std::string, std::vector<DebugTraceEvent>> s;
        return s;
    }

    static std::map<std::string, DebugChecklistTemplate>& checklistBySession() {
        static std::map<std::string, DebugChecklistTemplate> s;
        return s;
    }

    static std::map<std::string, DebugSessionHandoff>& handoffBySession() {
        static std::map<std::string, DebugSessionHandoff> s;
        return s;
    }
