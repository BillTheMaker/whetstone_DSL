#pragma once
// Step 1158: Sprint 92 integration summary.
#include <nlohmann/json.hpp>

struct Sprint92IntegrationSummary {
    static constexpr int sprintNumber = 92;
    static constexpr int stepsCompleted = 10;
    static constexpr const char* theme = "Interop Testbeds and Reference Implementations";
    static bool verify() { return sprintNumber == 92 && stepsCompleted == 10; }
    static nlohmann::json toJson() {
        return {{"sprint", sprintNumber},
                {"steps", stepsCompleted},
                {"theme", theme},
                {"status", "complete"},
                {"tools_added", {"whetstone_run_interop_testbed", "whetstone_get_reference_conformance_report"}},
                {"components", {"InteropTestbedManifestModel", "ReferenceImplementationHarnessForIREvidenceSpecs",
                                 "ThirdPartyConformanceReplayRunner", "CanonicalFixtureAndOracleDatasetPacks",
                                 "DivergenceTriageModelForInteropFailures", "InteropCertificationPolicyBindings",
                                 "InteropPublicationBundle"}}};
    }
};
