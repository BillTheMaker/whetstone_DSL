// Sprint 53 managed family MCP tool (Step 766)
// Included inside MCPServer class body.

    void registerManagedFamilyTools() {
        tools_.push_back({"whetstone_transpile_managed_family",
            "Lower Kotlin/C#/F#/VB.NET source, bridge nullability/async/ADT intent, and raise into managed-family targets.",
            {{"type", "object"}, {"properties", {
                {"source_language", {{"type", "string"}}},
                {"target_language", {{"type", "string"}}},
                {"source", {{"type", "string"}}},
                {"profile", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"source_language", "target_language", "source"})}}
        });
        toolHandlers_["whetstone_transpile_managed_family"] =
            [this](const nlohmann::json& args) { return runTranspileManagedFamily(args); };
    }

    nlohmann::json runTranspileManagedFamily(const nlohmann::json& args) {
        if (!args.contains("source_language") || !args["source_language"].is_string()) {
            return {{"success", false}, {"error", "source_language_missing"}};
        }
        if (!args.contains("target_language") || !args["target_language"].is_string()) {
            return {{"success", false}, {"error", "target_language_missing"}};
        }
        if (!args.contains("source") || !args["source"].is_string()) {
            return {{"success", false}, {"error", "source_missing"}};
        }

        const std::string srcLang = args.value("source_language", "");
        const std::string tgtLang = args.value("target_language", "");
        const std::string source = args.value("source", "");
        const std::string profile = args.value("profile", "balanced");

        ManagedLoweringPacket lower;
        if (srcLang == "kotlin") lower = KotlinAdapterV1::lower(source);
        else if (srcLang == "csharp") lower = CSharpAdapterV1::lower(source);
        else if (srcLang == "fsharp") lower = FSharpAdapterV1::lower(source);
        else if (srcLang == "vbnet") lower = VbNetAdapterV1::lower(source);
        else return {{"success", false}, {"error", "unsupported_source_language"}};

        ManagedRaisingPacket raise;
        if (tgtLang == "kotlin") raise = KotlinAdapterV1::raise(lower.irSummary, profile);
        else if (tgtLang == "csharp") raise = CSharpAdapterV1::raise(lower.irSummary, profile);
        else if (tgtLang == "fsharp") raise = FSharpAdapterV1::raise(lower.irSummary, profile);
        else if (tgtLang == "vbnet") raise = VbNetAdapterV1::raise(lower.irSummary, profile);
        else return {{"success", false}, {"error", "unsupported_target_language"}};

        auto nullability = NullabilityOptionalityBridge::fromLowering(lower);
        auto async = AsyncModelBridge::plan(srcLang, tgtLang, lower);
        auto adt = ADTPatternCanonicalLowering::lower(source);
        std::string tier = ManagedFamilyPromotionMatrix::lookupTier(srcLang, tgtLang);

        return {
            {"success", true},
            {"tier", tier},
            {"lowering", {
                {"source_language", lower.sourceLanguage},
                {"ir_summary", lower.irSummary},
                {"has_nullable_syntax", lower.hasNullableSyntax},
                {"has_optional_type", lower.hasOptionalType},
                {"async_signal_count", lower.asyncSignalCount},
                {"adt_like", lower.adtLike}
            }},
            {"raising", {
                {"target_language", raise.targetLanguage},
                {"code_preview", raise.codePreview},
                {"nullability_model", raise.nullabilityModel},
                {"async_model", raise.asyncModel}
            }},
            {"nullability_bridge", NullabilityOptionalityBridge::toJson(nullability)},
            {"async_bridge", AsyncModelBridge::toJson(async)},
            {"adt_bridge", ADTPatternCanonicalLowering::toJson(adt)}
        };
    }
