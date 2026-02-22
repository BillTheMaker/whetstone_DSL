#pragma once
// Step 808: Sprint 57 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "low_level/AbiCallingConventionModel.h"
#include "low_level/CInteropAdapter.h"
#include "low_level/WasmAdapterV2.h"
#include "low_level/X86AdapterV1.h"
#include "low_level/ARMAdapterV1.h"
#include "low_level/MemoryLayoutChecker.h"
#include "low_level/HostBoundaryContract.h"
#include "low_level/LowLevelAcceptanceReport.h"
#include "MCPServer.h"

struct Sprint57IntegrationResult {
    bool abiReady = false;
    bool cReady = false;
    bool wasmReady = false;
    bool x86Ready = false;
    bool armReady = false;
    bool layoutReady = false;
    bool contractReady = false;
    bool acceptanceReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 799;
    int stepEnd = 808;
    std::vector<std::string> filesAdded;
};

class Sprint57IntegrationSummary {
public:
    static Sprint57IntegrationResult run() {
        Sprint57IntegrationResult out;
        out.filesAdded = {
            "low_level/AbiCallingConventionModel.h",
            "low_level/CInteropAdapter.h",
            "low_level/WasmAdapterV2.h",
            "low_level/X86AdapterV1.h",
            "low_level/ARMAdapterV1.h",
            "low_level/MemoryLayoutChecker.h",
            "low_level/HostBoundaryContract.h",
            "low_level/LowLevelAcceptanceReport.h",
            "mcp/RegisterLowLevelFamilyTools.h",
            "Sprint57IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto abi = AbiCallingConventionModel::describe("import func","c");
        auto c = CInteropAdapter::describe("void foo();");
        auto was = WasmAdapterV2::describe("host imported func");
        auto x86 = X86AdapterV1::describe("fastcall entry");
        auto arm = ARMAdapterV1::describe("aapcs packed");

        out.abiReady = !abi.callingConvention.empty();
        out.cReady = c.language == "c";
        out.wasmReady = was.language == "wasm";
        out.x86Ready = x86.language == "x86";
        out.armReady = arm.language == "arm";

        auto layout = MemoryLayoutChecker::assess(c, arm);
        out.layoutReady = !layout.layout.empty();

        auto contract = HostBoundaryContract::build(AbiCallingConventionModel::describe("import","c"));
        out.contractReady = contract.needsReview;

        LowLevelEntry entry{contract.needsReview, false, "layout_mismatch"};
        auto report = LowLevelAcceptanceReportModel::build({entry});
        out.acceptanceReady = report.total == 1;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_transpile_low_level_family") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.abiReady && out.cReady && out.wasmReady && out.x86Ready && out.armReady &&
                      out.layoutReady && out.contractReady && out.acceptanceReady && out.mcpToolReady &&
                      out.stepStart == 799 && out.stepEnd == 808;
        return out;
    }
};
