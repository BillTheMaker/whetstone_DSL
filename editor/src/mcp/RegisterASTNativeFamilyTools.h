// Sprint 54 AST-native family MCP tool (Step 776)
// Included inside MCPServer class body.

    void registerASTNativeFamilyTools() {
        tools_.push_back({"whetstone_transpile_ast_native_family",
            "Lower Lisp/Scheme/Elisp/Smalltalk source into canonical AST-native IR and project to target language.",
            {{"type", "object"}, {"properties", {
                {"source_language", {{"type", "string"}}},
                {"target_language", {{"type", "string"}}},
                {"source", {{"type", "string"}}},
                {"profile", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"source_language", "target_language", "source"})}}
        });
        toolHandlers_["whetstone_transpile_ast_native_family"] =
            [this](const nlohmann::json& args) { return runTranspileASTNativeFamily(args); };
    }

    nlohmann::json runTranspileASTNativeFamily(const nlohmann::json& args) {
        if (!args.contains("source_language") || !args["source_language"].is_string()) return {{"success", false}, {"error", "source_language_missing"}};
        if (!args.contains("target_language") || !args["target_language"].is_string()) return {{"success", false}, {"error", "target_language_missing"}};
        if (!args.contains("source") || !args["source"].is_string()) return {{"success", false}, {"error", "source_missing"}};

        const std::string srcLang = args.value("source_language", "");
        const std::string tgtLang = args.value("target_language", "");
        const std::string source = args.value("source", "");
        const std::string profile = args.value("profile", "canonical");

        ASTNativeLoweringPacket lower;
        if (srcLang == "lisp") lower = LispAdapterV1::lower(source);
        else if (srcLang == "scheme") lower = SchemeAdapterV1::lower(source);
        else if (srcLang == "elisp") lower = ElispAdapterV1::lower(source);
        else if (srcLang == "smalltalk") lower = SmalltalkAdapterV1::lower(source);
        else return {{"success", false}, {"error", "unsupported_source_language"}};

        ASTNativeRaisingPacket raise;
        if (tgtLang == "lisp") raise = LispAdapterV1::raise(lower.canonicalForm, profile);
        else if (tgtLang == "scheme") raise = SchemeAdapterV1::raise(lower.canonicalForm, profile);
        else if (tgtLang == "elisp") raise = ElispAdapterV1::raise(lower.canonicalForm, profile);
        else if (tgtLang == "smalltalk") raise = SmalltalkAdapterV1::raise(lower.canonicalForm, profile);
        else return {{"success", false}, {"error", "unsupported_target_language"}};

        auto macro = MacroHygieneBoundaryModel::classify(lower, srcLang);
        auto evalRisk = EvalRuntimeRiskModel::classify(source);
        int reviewRequired = (macro.expansionRisk > 1 || evalRisk.level == "high") ? 1 : 0;
        auto benchmark = ASTNativeProjectionBenchmarkSuite::run(1, reviewRequired);

        return {
            {"success", true},
            {"lowering", SExpressionCanonicalLowering::toJson(lower)},
            {"raising", SExpressionCanonicalLowering::toJson(raise)},
            {"macro_boundary", MacroHygieneBoundaryModel::toJson(macro)},
            {"eval_risk", EvalRuntimeRiskModel::toJson(evalRisk)},
            {"benchmark", ASTNativeProjectionBenchmarkSuite::toJson(benchmark)}
        };
    }
