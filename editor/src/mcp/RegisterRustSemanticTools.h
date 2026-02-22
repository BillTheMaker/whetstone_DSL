// Sprint 47 rust semantic lowering MCP tool (Step 707)
// Included inside MCPServer class body.

    void registerRustSemanticTools() {
        tools_.push_back({"whetstone_analyze_rust_semantics",
            "Analyze Rust source into semantic lowering packets for ownership, lifetimes, traits, generics, errors, async, macros, and unsafe risk.",
            {{"type", "object"}, {"properties", {
                {"source", {{"type", "string"}, {"description", "Rust source code to analyze."}}},
                {"strict", {{"type", "boolean"}, {"description", "Fail closed on unsupported macro boundaries."}}}
            }}, {"required", nlohmann::json::array({"source"})}}
        });
        toolHandlers_["whetstone_analyze_rust_semantics"] =
            [this](const nlohmann::json& args) { return runAnalyzeRustSemantics(args); };
    }

    nlohmann::json runAnalyzeRustSemantics(const nlohmann::json& args) {
        if (!args.contains("source") || !args["source"].is_string()) {
            return {{"success", false}, {"error", "source_missing"}};
        }

        const std::string source = args.value("source", "");
        const bool strict = args.value("strict", false);

        auto ownership = RustOwnershipBorrowExtractor::extract(source);
        auto lifetimes = RustLifetimeRegionLowering::lower(source);
        auto traits = RustTraitImplLowering::lower(source);
        auto generics = RustGenericIntentModel::lower(source);
        auto errors = RustErrorModelLowering::lower(source);
        auto async = RustAsyncIntentLowering::lower(source);
        auto macros = RustMacroBoundaryPolicy::analyze(source);
        auto unsafe = RustUnsafeRiskPacketGenerator::analyze(source);

        bool reviewRequired = unsafe.reviewRequired;
        for (const auto& m : macros) {
            if (!m.supported) reviewRequired = true;
            if (strict && !m.supported) {
                return {
                    {"success", false},
                    {"error", "unsupported_macro_boundary"},
                    {"macro", m.macroName}
                };
            }
        }

        std::string err;
        if (!RustOwnershipBorrowExtractor::validate(ownership, &err)) {
            return {{"success", false}, {"error", err}};
        }

        return {
            {"success", true},
            {"reviewRequired", reviewRequired},
            {"ownership", RustOwnershipBorrowExtractor::toJson(ownership)},
            {"lifetimes", RustLifetimeRegionLowering::toJson(lifetimes)},
            {"traits", RustTraitImplLowering::toJson(traits)},
            {"generics", RustGenericIntentModel::toJson(generics)},
            {"errorModel", RustErrorModelLowering::toJson(errors)},
            {"asyncModel", RustAsyncIntentLowering::toJson(async)},
            {"macroBoundaries", RustMacroBoundaryPolicy::toJson(macros)},
            {"unsafeRisk", RustUnsafeRiskPacketGenerator::toJson(unsafe)}
        };
    }
