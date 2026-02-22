// Sprint 52 dynamic family MCP tool (Step 756)
// Included inside MCPServer class body.

    void registerDynamicFamilyTools() {
        tools_.push_back({"whetstone_transpile_dynamic_family",
            "Lower dynamic-family source and produce strictness/risk packets for static projection.",
            {{"type", "object"}, {"properties", {
                {"source_language", {{"type", "string"}}},
                {"source", {{"type", "string"}}},
                {"strictness", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"source_language", "source"})}}
        });
        toolHandlers_["whetstone_transpile_dynamic_family"] =
            [this](const nlohmann::json& args) { return runTranspileDynamicFamily(args); };
    }

    nlohmann::json runTranspileDynamicFamily(const nlohmann::json& args) {
        if (!args.contains("source_language") || !args["source_language"].is_string()) {
            return {{"success", false}, {"error", "source_language_missing"}};
        }
        if (!args.contains("source") || !args["source"].is_string()) {
            return {{"success", false}, {"error", "source_missing"}};
        }
        const std::string lang = args.value("source_language", "");
        const std::string source = args.value("source", "");
        const std::string strictness = args.value("strictness", "balanced");

        DynamicLoweringPacket lower;
        if (lang == "python") lower = PythonLoweringAdapterV2::lower(source);
        else if (lang == "javascript") lower = JavaScriptLoweringAdapterV2::lower(source);
        else if (lang == "typescript") lower = TypeScriptLoweringAdapterV2::lower(source);
        else if (lang == "ruby") lower = RubyLoweringAdapterV1::lower(source);
        else if (lang == "lua") lower = LuaLoweringAdapterV1::lower(source);
        else return {{"success", false}, {"error", "unsupported_source_language"}};

        auto policy = DynamicStrictnessPolicyEngine::forMode(strictness);
        bool reflection = source.find("reflect") != std::string::npos || source.find("method_missing") != std::string::npos;
        auto risk = DynamicRiskClassifier::classify(lower.dynamicDispatchCount, reflection, policy);

        auto report = DynamicFamilyAcceptanceReportModel::build({lower}, {risk});

        return {
            {"success", true},
            {"lowering", {
                {"source_language", lower.sourceLanguage},
                {"ir_summary", lower.irSummary},
                {"dynamic_dispatch_count", lower.dynamicDispatchCount},
                {"review_required", lower.reviewRequired}
            }},
            {"strictness_policy", DynamicStrictnessPolicyEngine::toJson(policy)},
            {"risk", DynamicRiskClassifier::toJson(risk)},
            {"acceptance", DynamicFamilyAcceptanceReportModel::toJson(report)}
        };
    }
