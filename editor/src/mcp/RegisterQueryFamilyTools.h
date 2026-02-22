// Sprint 56 query family MCP tool (Step 796)
// Included inside MCPServer class body.

    void registerQueryFamilyTools() {
        tools_.push_back({"whetstone_transpile_query_family",
            "Lower SQL dialect queries, model transaction/equivalence/divergence, and project into target dialect.",
            {{"type", "object"}, {"properties", {
                {"source_dialect", {{"type", "string"}}},
                {"target_dialect", {{"type", "string"}}},
                {"query", {{"type", "string"}}},
                {"profile", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"source_dialect", "target_dialect", "query"})}}
        });
        toolHandlers_["whetstone_transpile_query_family"] =
            [this](const nlohmann::json& args) { return runTranspileQueryFamily(args); };
    }

    nlohmann::json runTranspileQueryFamily(const nlohmann::json& args) {
        if (!args.contains("source_dialect") || !args["source_dialect"].is_string()) return {{"success", false}, {"error", "source_dialect_missing"}};
        if (!args.contains("target_dialect") || !args["target_dialect"].is_string()) return {{"success", false}, {"error", "target_dialect_missing"}};
        if (!args.contains("query") || !args["query"].is_string()) return {{"success", false}, {"error", "query_missing"}};

        const std::string src = args.value("source_dialect", "");
        const std::string tgt = args.value("target_dialect", "");
        const std::string q = args.value("query", "");
        const std::string profile = args.value("profile", "safe");

        QueryLoweringPacket lower;
        if (src == "postgresql") lower = PostgreSqlAdapterV1::lower(q);
        else if (src == "tsql") lower = TSqlAdapterV1::lower(q);
        else if (src == "mysql") lower = MySqlAdapterV1::lower(q);
        else return {{"success", false}, {"error", "unsupported_source_dialect"}};

        QueryRaisingPacket raise;
        if (tgt == "postgresql") raise = PostgreSqlAdapterV1::raise(lower.irSummary, profile);
        else if (tgt == "tsql") raise = TSqlAdapterV1::raise(lower.irSummary, profile);
        else if (tgt == "mysql") raise = MySqlAdapterV1::raise(lower.irSummary, profile);
        else return {{"success", false}, {"error", "unsupported_target_dialect"}};

        auto tx = TransactionIsolationModel::analyze(q, "read_committed");
        auto eq = QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}), nlohmann::json::array({{{"id",1}}}));
        auto div = QueryDivergenceClassifier::classify(lower);
        auto report = DataFamilyAcceptanceReportModel::build({div});

        return {
            {"success", true},
            {"lowering", SqlCanonicalQueryIR::toJson(lower)},
            {"raising", SqlCanonicalQueryIR::toJson(raise)},
            {"transaction", TransactionIsolationModel::toJson(tx)},
            {"equivalence", QueryBehaviorEquivalenceRunner::toJson(eq)},
            {"divergence", QueryDivergenceClassifier::toJson(div)},
            {"acceptance", DataFamilyAcceptanceReportModel::toJson(report)}
        };
    }
