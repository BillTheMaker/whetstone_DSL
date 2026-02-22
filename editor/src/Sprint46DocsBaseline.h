#pragma once
// Step 696: baseline docs + examples for Sprint 46 foundations.

#include <string>
#include <vector>

#include "LanguageSupportTier.h"
#include "MigrationAcceptanceContract.h"
#include "SemanticCoreIR.h"

class Sprint46DocsBaseline {
public:
    static std::vector<std::string> docPaths() {
        return {
            "docs/SPRINT46_FOUNDATION.md",
            "docs/examples/sprint46_semantic_core_example.json"
        };
    }

    static SemanticCoreIR exampleIr() {
        SemanticCoreIR ir;
        ir.moduleId = "example-mod";
        ir.moduleName = "Example";
        ir.nodes.push_back({"m1", IRNodeKind::Module, "Example", "rust", {"stateful"}, json::object(), json::object()});
        ir.nodes.push_back({"f1", IRNodeKind::Function, "run", "rust", {"algorithmic"}, json::object(), json::object()});
        ir.edges.push_back({"m1", "f1", "contains"});
        ir.contracts = {{"preconditions", {"input_valid"}}, {"postconditions", {"output_nonempty"}}};
        return ir;
    }

    static json exampleContractPacket() {
        MigrationGateThresholds th;
        return {
            {"minimumTestPassRate", th.minimumTestPassRate},
            {"maxHighSeverityFindings", th.maxHighSeverityFindings},
            {"maxPerfRegressionPct", th.maxPerfRegressionPct},
            {"requireChecklistComplete", th.requireChecklistComplete}
        };
    }
};
