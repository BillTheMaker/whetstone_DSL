#pragma once
// Step 728: Sprint 49 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "equiv/DifferentialExecutionHarness.h"
#include "equiv/TestVectorSpec.h"
#include "equiv/RustRunnerAdapter.h"
#include "equiv/CppRunnerAdapter.h"
#include "equiv/PropertyEquivalenceRunner.h"
#include "equiv/FuzzDifferentialRunner.h"
#include "equiv/BehavioralDivergencePacket.h"
#include "equiv/EquivalenceEvidenceBundle.h"

struct Sprint49IntegrationResult {
    bool harnessReady = false;
    bool vectorReady = false;
    bool rustRunnerReady = false;
    bool cppRunnerReady = false;
    bool propertyReady = false;
    bool fuzzReady = false;
    bool divergenceReady = false;
    bool evidenceReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 719;
    int stepEnd = 728;
    std::vector<std::string> filesAdded;
};

class Sprint49IntegrationSummary {
public:
    static Sprint49IntegrationResult run() {
        Sprint49IntegrationResult out;
        out.filesAdded = {
            "equiv/DifferentialExecutionHarness.h",
            "equiv/TestVectorSpec.h",
            "equiv/RustRunnerAdapter.h",
            "equiv/CppRunnerAdapter.h",
            "equiv/PropertyEquivalenceRunner.h",
            "equiv/FuzzDifferentialRunner.h",
            "equiv/BehavioralDivergencePacket.h",
            "equiv/EquivalenceEvidenceBundle.h",
            "mcp/RegisterEquivalenceTools.h",
            "Sprint49IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto vectors = TestVectorSpec::normalize({{"v2", {"x", 2}, "ok"}, {"v1", {"x", 1}, "ok"}});
        out.vectorReady = vectors.size() == 2 && vectors[0].id == "v1";

        auto rust = RustRunnerAdapter::run(vectors);
        auto cpp = CppRunnerAdapter::run(vectors);
        out.rustRunnerReady = rust.outputs.size() == vectors.size();
        out.cppRunnerReady = cpp.outputs.size() == vectors.size();

        std::vector<DifferentialCaseResult> cases;
        for (size_t i = 0; i < vectors.size(); ++i) cases.push_back({vectors[i].id, rust.outputs[i], cpp.outputs[i], false});
        auto diff = DifferentialExecutionHarness::run(cases);
        out.harnessReady = diff.totalCount == 2 && diff.equivalentCount == 2;

        auto prop = PropertyEquivalenceRunner::run(8, 42);
        out.propertyReady = prop.success;

        auto fuzz = FuzzDifferentialRunner::run({3, 2, 1});
        out.fuzzReady = !fuzz.empty() && fuzz[0].seed == 1;

        auto div = BehavioralDivergencePacketModel::classify("c1", "a", "b", "long_input_value_for_minimize");
        out.divergenceReady = div.classification == "unacceptable_divergence";

        out.evidenceReady = EquivalenceEvidenceBundleModel::toJson(EquivalenceEvidenceBundleModel::build(diff, prop, fuzz)).contains("differential");

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_verify_executable_equivalence") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.harnessReady && out.vectorReady && out.rustRunnerReady && out.cppRunnerReady && out.propertyReady &&
                      out.fuzzReady && out.divergenceReady && out.evidenceReady && out.mcpToolReady &&
                      out.stepStart == 719 && out.stepEnd == 728;
        return out;
    }
};
