// Sprint 51 systems family MCP tool (Step 746)
// Included inside MCPServer class body.

    void registerSystemsFamilyTools() {
        tools_.push_back({"whetstone_transpile_systems_family",
            "Lower C/Go/Java source to IR and raise into selected systems-language target.",
            {{"type", "object"}, {"properties", {
                {"source_language", {{"type", "string"}}},
                {"target_language", {{"type", "string"}}},
                {"source", {{"type", "string"}}},
                {"profile", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"source_language", "target_language", "source"})}}
        });
        toolHandlers_["whetstone_transpile_systems_family"] =
            [this](const nlohmann::json& args) { return runTranspileSystemsFamily(args); };
    }

    nlohmann::json runTranspileSystemsFamily(const nlohmann::json& args) {
        if (!args.contains("source_language") || !args["source_language"].is_string()) return {{"success", false}, {"error", "source_language_missing"}};
        if (!args.contains("target_language") || !args["target_language"].is_string()) return {{"success", false}, {"error", "target_language_missing"}};
        if (!args.contains("source") || !args["source"].is_string()) return {{"success", false}, {"error", "source_missing"}};

        const std::string srcLang = args.value("source_language", "");
        const std::string tgtLang = args.value("target_language", "");
        const std::string source = args.value("source", "");
        const std::string profile = args.value("profile", "safe-first");

        LoweringPacket lowered;
        if (srcLang == "c") lowered = CLoweringAdapterV1::lower(source);
        else if (srcLang == "go") lowered = GoLoweringAdapterV1::lower(source);
        else if (srcLang == "java") lowered = JavaLoweringAdapterV1::lower(source);
        else return {{"success", false}, {"error", "unsupported_source_language"}};

        RaisingPacket raised;
        if (tgtLang == "c") raised = CRaisingAdapterV1::raise(lowered.irSummary, profile);
        else if (tgtLang == "go") raised = GoRaisingAdapterV1::raise(lowered.irSummary, profile);
        else if (tgtLang == "java") raised = JavaRaisingAdapterV1::raise(lowered.irSummary, profile);
        else return {{"success", false}, {"error", "unsupported_target_language"}};

        auto pairs = SystemsCompatibilityMatrix::defaultPairs();
        bool supported = false;
        std::string tier = "experimental";
        for (const auto& p : pairs) {
            if (p.source == srcLang && p.target == tgtLang) {
                supported = true;
                tier = p.tier;
                break;
            }
        }

        return {
            {"success", true},
            {"supported_pair", supported},
            {"tier", tier},
            {"lowering", {
                {"source_language", lowered.sourceLanguage},
                {"ir_summary", lowered.irSummary},
                {"confidence", lowered.confidence}
            }},
            {"raising", {
                {"target_language", raised.targetLanguage},
                {"code_preview", raised.codePreview},
                {"profile", raised.profile}
            }}
        };
    }
