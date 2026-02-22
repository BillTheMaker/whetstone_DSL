// Sprint 55 logic+actor family MCP tool (Step 786)
// Included inside MCPServer class body.

    void registerLogicActorFamilyTools() {
        tools_.push_back({"whetstone_transpile_logic_actor_family",
            "Lower Prolog/Erlang/Elixir source and apply logic+actor projection policies with semantic safety gates.",
            {{"type", "object"}, {"properties", {
                {"source_language", {{"type", "string"}}},
                {"target_language", {{"type", "string"}}},
                {"source", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"source_language", "target_language", "source"})}}
        });
        toolHandlers_["whetstone_transpile_logic_actor_family"] =
            [this](const nlohmann::json& args) { return runTranspileLogicActorFamily(args); };
    }

    nlohmann::json runTranspileLogicActorFamily(const nlohmann::json& args) {
        if (!args.contains("source_language") || !args["source_language"].is_string()) return {{"success", false}, {"error", "source_language_missing"}};
        if (!args.contains("target_language") || !args["target_language"].is_string()) return {{"success", false}, {"error", "target_language_missing"}};
        if (!args.contains("source") || !args["source"].is_string()) return {{"success", false}, {"error", "source_missing"}};

        const std::string srcLang = args.value("source_language", "");
        const std::string tgtLang = args.value("target_language", "");
        const std::string source = args.value("source", "");

        LogicActorLoweringPacket lower;
        if (srcLang == "prolog") lower = PrologAdapterV1::lower(source);
        else if (srcLang == "erlang") lower = ErlangAdapterV1::lower(source);
        else if (srcLang == "elixir") lower = ElixirAdapterV1::lower(source);
        else return {{"success", false}, {"error", "unsupported_source_language"}};

        auto logicPolicy = LogicImperativeProjectionPolicy::choose(lower, tgtLang);
        auto actorPolicy = ActorAsyncProjectionPolicy::choose(lower, tgtLang);
        auto supervision = SupervisionTreePacketModel::build(lower, tgtLang);
        auto gate = SemanticBlocklistGate::evaluate(lower, tgtLang);
        auto report = LogicActorAcceptanceReportModel::build({gate});

        return {
            {"success", true},
            {"lowering", {
                {"source_language", lower.sourceLanguage},
                {"ir_summary", lower.irSummary},
                {"query_arity", lower.queryArity},
                {"backtracking", lower.backtracking},
                {"actor_model", lower.actorModel},
                {"supervision", lower.supervision}
            }},
            {"logic_policy", LogicImperativeProjectionPolicy::toJson(logicPolicy)},
            {"actor_policy", ActorAsyncProjectionPolicy::toJson(actorPolicy)},
            {"supervision_packet", SupervisionTreePacketModel::toJson(supervision)},
            {"semantic_gate", SemanticBlocklistGate::toJson(gate)},
            {"acceptance", LogicActorAcceptanceReportModel::toJson(report)}
        };
    }
