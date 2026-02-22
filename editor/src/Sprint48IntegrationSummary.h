#pragma once
// Step 718: Sprint 48 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "cpp_ir/CppOwnershipMappingPolicy.h"
#include "cpp_ir/CppBorrowMappingStrategy.h"
#include "cpp_ir/CppTraitRaising.h"
#include "cpp_ir/CppTemplateRaisingPolicy.h"
#include "cpp_ir/CppErrorModelMapping.h"
#include "cpp_ir/CppAsyncMappingStrategy.h"
#include "cpp_ir/CppAlgorithmLifting.h"
#include "cpp_ir/CppBuildArtifactGenerator.h"

struct Sprint48IntegrationResult {
    bool ownershipReady = false;
    bool borrowReady = false;
    bool traitReady = false;
    bool templateReady = false;
    bool errorReady = false;
    bool asyncReady = false;
    bool algorithmReady = false;
    bool buildReady = false;
    bool mcpToolReady = false;

    int stepStart = 709;
    int stepEnd = 718;
    std::vector<std::string> filesAdded;
    bool success = false;
};

class Sprint48IntegrationSummary {
public:
    static SemanticCoreIR sampleIr() {
        SemanticCoreIR ir;
        ir.moduleId = "m48";
        ir.moduleName = "Sprint48";
        ir.nodes.push_back({"n_mod", IRNodeKind::Module, "M", "rust", {"stateful"}, nlohmann::json::object(), nlohmann::json::object()});
        ir.nodes.push_back({"n_type", IRNodeKind::Type, "Drawable", "rust", {"shared"}, nlohmann::json{{"trait", true}}, nlohmann::json::object()});
        ir.nodes.push_back({"n_fn", IRNodeKind::Function, "run", "rust", {"algorithmic", "async"}, nlohmann::json::object(), nlohmann::json{{"genericParams", nlohmann::json::array({"T"})}}});
        ir.nodes.push_back({"n_own", IRNodeKind::OwnershipRegion, "owner", "rust", {"stateful"}, nlohmann::json::object(), nlohmann::json::object()});
        ir.nodes.push_back({"n_conc", IRNodeKind::ConcurrencyRegion, "task", "rust", {"async"}, nlohmann::json::object(), nlohmann::json::object()});
        ir.edges.push_back({"n_mod", "n_fn", "contains"});
        ir.edges.push_back({"n_fn", "n_own", "borrows"});
        ir.contracts = {{"errorBehavior", nlohmann::json::array({"panic_possible"})}};
        return ir;
    }

    static Sprint48IntegrationResult run() {
        Sprint48IntegrationResult out;
        out.filesAdded = {
            "CppOwnershipMappingPolicy.h",
            "CppBorrowMappingStrategy.h",
            "CppTraitRaising.h",
            "CppTemplateRaisingPolicy.h",
            "CppErrorModelMapping.h",
            "CppAsyncMappingStrategy.h",
            "CppAlgorithmLifting.h",
            "CppBuildArtifactGenerator.h",
            "RegisterCppRaisingTools.h",
            "Sprint48IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto ir = sampleIr();
        out.ownershipReady = !CppOwnershipMappingPolicy::map(ir).empty();
        out.borrowReady = !CppBorrowMappingStrategy::map(ir).empty();
        out.traitReady = !CppTraitRaising::raise(ir).empty();
        out.templateReady = !CppTemplateRaisingPolicy::raise(ir).empty();
        out.errorReady = !CppErrorModelMapping::map(ir).resultType.empty();
        out.asyncReady = !CppAsyncMappingStrategy::map(ir).empty();
        out.algorithmReady = !CppAlgorithmLifting::lift(ir).empty();
        out.buildReady = !CppBuildArtifactGenerator::generate("demo").cmakeLists.empty();

        MCPServer mcp;
        nlohmann::json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}};
        auto resp = mcp.handleRequest(req);
        for (const auto& t : resp["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_generate_cpp_from_ir") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.ownershipReady && out.borrowReady && out.traitReady && out.templateReady &&
                      out.errorReady && out.asyncReady && out.algorithmReady && out.buildReady &&
                      out.mcpToolReady && out.stepStart == 709 && out.stepEnd == 718;
        return out;
    }
};
