#pragma once
// Step 708: Sprint 47 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "rust_ir/RustOwnershipBorrowExtractor.h"
#include "rust_ir/RustLifetimeRegionLowering.h"
#include "rust_ir/RustTraitImplLowering.h"
#include "rust_ir/RustGenericIntentModel.h"
#include "rust_ir/RustErrorModelLowering.h"
#include "rust_ir/RustAsyncIntentLowering.h"
#include "rust_ir/RustMacroBoundaryPolicy.h"
#include "rust_ir/RustUnsafeRiskPacket.h"

struct Sprint47IntegrationResult {
    bool ownershipReady = false;
    bool lifetimeReady = false;
    bool traitReady = false;
    bool genericReady = false;
    bool errorReady = false;
    bool asyncReady = false;
    bool macroReady = false;
    bool unsafeReady = false;
    bool mcpToolReady = false;

    int stepStart = 699;
    int stepEnd = 708;
    std::vector<std::string> filesAdded;

    bool success = false;
};

class Sprint47IntegrationSummary {
public:
    static Sprint47IntegrationResult run() {
        const std::string sample =
            "trait Draw { fn draw(&self); }\n"
            "impl Draw for Canvas { fn draw(&self) {} }\n"
            "async fn run<'a, T>(x: &'a T) -> Result<Option<i32>, String> {\n"
            "  println!(\"hi\");\n"
            "  panic!(\"x\");\n"
            "}\n"
            "unsafe { let p: *const i32 = core::ptr::null(); }\n";

        Sprint47IntegrationResult out;
        out.filesAdded = {
            "RustOwnershipBorrowExtractor.h",
            "RustLifetimeRegionLowering.h",
            "RustTraitImplLowering.h",
            "RustGenericIntentModel.h",
            "RustErrorModelLowering.h",
            "RustAsyncIntentLowering.h",
            "RustMacroBoundaryPolicy.h",
            "RustUnsafeRiskPacket.h",
            "RegisterRustSemanticTools.h",
            "Sprint47IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        out.ownershipReady = !RustOwnershipBorrowExtractor::extract("let a = 1; let b = a;").regions.empty();
        out.lifetimeReady = !RustLifetimeRegionLowering::lower("fn f<'a>(x: &'a i32) {}").empty();
        out.traitReady = !RustTraitImplLowering::lower(sample).traits.empty();
        out.genericReady = !RustGenericIntentModel::lower("fn g<T>(x:T){}").empty();
        out.errorReady = RustErrorModelLowering::lower(sample).usesResult;
        out.asyncReady = !RustAsyncIntentLowering::lower(sample).empty();
        out.macroReady = !RustMacroBoundaryPolicy::analyze(sample).empty();
        out.unsafeReady = RustUnsafeRiskPacketGenerator::analyze(sample).reviewRequired;

        MCPServer mcp;
        nlohmann::json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}};
        auto resp = mcp.handleRequest(req);
        for (const auto& t : resp["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_analyze_rust_semantics") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.ownershipReady && out.lifetimeReady && out.traitReady && out.genericReady &&
                      out.errorReady && out.asyncReady && out.macroReady && out.unsafeReady &&
                      out.mcpToolReady && out.stepStart == 699 && out.stepEnd == 708;
        return out;
    }
};
