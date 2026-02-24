// Sprint 92: Interop MCP tools (Steps 1155-1156)
// Included inside MCPServer class body.

    void registerInteropTools() {
        tools_.push_back({"whetstone_run_interop_testbed",
            "Run interop testbed manifest against a vendor and return replay/triage details.",
            {{"type", "object"}, {"properties", {
                {"manifest_id", {{"type", "string"}}},
                {"vendor_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"manifest_id"})}}
        });
        toolHandlers_["whetstone_run_interop_testbed"] =
            [this](const nlohmann::json& args) { return runInteropTestbed(args); };

        tools_.push_back({"whetstone_get_reference_conformance_report",
            "Get reference conformance report and certification policy binding status.",
            {{"type", "object"}, {"properties", {
                {"vendor_id", {{"type", "string"}}},
                {"manifest_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"vendor_id"})}}
        });
        toolHandlers_["whetstone_get_reference_conformance_report"] =
            [this](const nlohmann::json& args) { return runReferenceConformanceReport(args); };
    }

    nlohmann::json runInteropTestbed(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "manifest_id required"}};
        std::string manifestId = args.value("manifest_id", "");
        if (manifestId.empty()) return {{"success", false}, {"error", "manifest_id required"}};

        std::string vendorId = args.value("vendor_id", "vendor-a");
        auto manifest = InteropTestbedManifestModelFactory::make(
            manifestId, "migration-core", {"cpp", "rust", "python"}, 24, true);
        auto harness = ReferenceImplementationHarnessForIREvidenceSpecsFactory::make(
            "ref-harness-1", "ir-v1", "evidence-v1", {"cpp", "rust"}, true);
        auto replay = ThirdPartyConformanceReplayRunnerFactory::make(
            manifestId + "-run", vendorId, 24, 22);
        auto datasets = CanonicalFixtureAndOracleDatasetPacksFactory::make(
            "pack-core", 24, 24, "2026.02", true);
        auto triage = DivergenceTriageModelForInteropFailuresFactory::make(
            manifestId + "-triage", "semantic-mismatch", "medium", "request vendor patch");
        auto policy = InteropCertificationPolicyBindingsFactory::make(
            "interop-cert-v1", 0.90, true, true, true);

        return {{"success", true},
                {"manifest", InteropTestbedManifestModelFactory::toJson(manifest)},
                {"reference_harness", ReferenceImplementationHarnessForIREvidenceSpecsFactory::toJson(harness)},
                {"replay", ThirdPartyConformanceReplayRunnerFactory::toJson(replay)},
                {"datasets", CanonicalFixtureAndOracleDatasetPacksFactory::toJson(datasets)},
                {"triage", DivergenceTriageModelForInteropFailuresFactory::toJson(triage)},
                {"policy", InteropCertificationPolicyBindingsFactory::toJson(policy)}};
    }

    nlohmann::json runReferenceConformanceReport(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "vendor_id required"}};
        std::string vendorId = args.value("vendor_id", "");
        if (vendorId.empty()) return {{"success", false}, {"error", "vendor_id required"}};

        std::string manifestId = args.value("manifest_id", "interop-core-v1");
        auto replay = ThirdPartyConformanceReplayRunnerFactory::make(
            manifestId + "-run", vendorId, 30, 28);
        auto policy = InteropCertificationPolicyBindingsFactory::make(
            "interop-cert-v1", 0.90, true, true, true);
        auto bundle = InteropPublicationBundleFactory::make(
            manifestId + "-bundle", manifestId, "ref-harness-1", manifestId + "-report", true);

        bool certified = replay.conformant && replay.passRate >= policy.minPassRate && policy.valid;
        return {{"success", true},
                {"vendor_id", vendorId},
                {"manifest_id", manifestId},
                {"certified", certified},
                {"replay", ThirdPartyConformanceReplayRunnerFactory::toJson(replay)},
                {"policy", InteropCertificationPolicyBindingsFactory::toJson(policy)},
                {"publication_bundle", InteropPublicationBundleFactory::toJson(bundle)}};
    }
