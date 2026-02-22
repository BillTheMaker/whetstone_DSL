// Sprint 59 governance MCP tool (Step 826)
// Included inside MCPServer class body.

    void registerGovernanceTools() {
        tools_.push_back({"whetstone_review_porting_decision",
            "Record a porting review decision with policy pack enforcement and ledger append.",
            {{"type", "object"}, {"properties", {
                {"issue_id", {{"type", "string"}}},
                {"reviewer", {{"type", "string"}}},
                {"decision", {{"type", "string"}}},
                {"rationale", {{"type", "string"}}},
                {"policy_pack", {{"type", "string"}}},
                {"waiver_scope", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"issue_id", "reviewer", "decision", "rationale"})}}
        });
        toolHandlers_["whetstone_review_porting_decision"] =
            [this](const nlohmann::json& args) { return runReviewPortingDecision(args); };
    }

    nlohmann::json runReviewPortingDecision(const nlohmann::json& args) {
        if (!args.contains("issue_id") || !args["issue_id"].is_string())
            return {{"success", false}, {"error", "issue_id_missing"}};
        if (!args.contains("reviewer") || !args["reviewer"].is_string())
            return {{"success", false}, {"error", "reviewer_missing"}};
        if (!args.contains("decision") || !args["decision"].is_string())
            return {{"success", false}, {"error", "decision_missing"}};
        if (!args.contains("rationale") || !args["rationale"].is_string())
            return {{"success", false}, {"error", "rationale_missing"}};

        const std::string issueId  = args.value("issue_id", "");
        const std::string reviewer = args.value("reviewer", "");
        const std::string decision = args.value("decision", "");
        const std::string rationale = args.value("rationale", "");
        const std::string policy    = args.value("policy_pack", "safe-first");
        const std::string waiverScope = args.value("waiver_scope", "");

        LedgerEntry entry;
        entry.entryId   = issueId + ":" + reviewer;
        entry.issueRef  = issueId;
        entry.reviewer  = reviewer;
        entry.decision  = decision;
        entry.rationale = rationale;
        entry.waiverRef = waiverScope.empty() ? "" : "waiver:" + waiverScope;

        std::string err;
        bool ok = governanceLedger().append(entry, &err);

        return {
            {"success", ok},
            {"error", err},
            {"issue_id", issueId},
            {"reviewer", reviewer},
            {"decision", decision},
            {"policy_pack", policy},
            {"ledger_summary", ReviewerDecisionLedger::toJson(governanceLedger().summarize())}
        };
    }

    static ReviewerDecisionLedger& governanceLedger() {
        static ReviewerDecisionLedger ledger;
        return ledger;
    }
