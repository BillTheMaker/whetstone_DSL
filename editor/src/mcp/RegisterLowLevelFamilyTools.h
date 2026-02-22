// Sprint 57 low-level family MCP tool (Step 806)
// Included inside MCPServer class body.

#include "low_level/AbiCallingConventionModel.h"
#include "low_level/CInteropAdapter.h"
#include "low_level/WasmAdapterV2.h"
#include "low_level/X86AdapterV1.h"
#include "low_level/ARMAdapterV1.h"
#include "low_level/MemoryLayoutChecker.h"
#include "low_level/HostBoundaryContract.h"
#include "low_level/LowLevelAcceptanceReport.h"

    void registerLowLevelFamilyTools() {
        tools_.push_back({"whetstone_transpile_low_level_family",
            "Lower C/assembly/WASM sources with ABI/memory layout awareness and emit validation artifacts.",
            {{"type", "object"}, {"properties", {
                {"source_language", {{"type", "string"}}},
                {"source", {{"type", "string"}}},
                {"target_language", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"source_language", "source", "target_language"})}}
        });
        toolHandlers_["whetstone_transpile_low_level_family"] =
            [this](const nlohmann::json& args) { return runTranspileLowLevelFamily(args); };
    }

    nlohmann::json runTranspileLowLevelFamily(const nlohmann::json& args) {
        if (!args.contains("source_language") || !args["source_language"].is_string()) return {{"success", false}, {"error", "source_language_missing"}};
        if (!args.contains("source") || !args["source"].is_string()) return {{"success", false}, {"error", "source_missing"}};
        if (!args.contains("target_language") || !args["target_language"].is_string()) return {{"success", false}, {"error", "target_language_missing"}};

        const std::string lang = args.value("source_language", "");
        const std::string source = args.value("source", "");
        const std::string target = args.value("target_language", "");

        AbiPacket packet;
        if (lang == "c") packet = CInteropAdapter::describe(source);
        else if (lang == "wasm") packet = WasmAdapterV2::describe(source);
        else if (lang == "x86") packet = X86AdapterV1::describe(source);
        else if (lang == "arm") packet = ARMAdapterV1::describe(source);
        else return {{"success", false}, {"error", "unsupported_source_language"}};

        AbiPacket targetPacket;
        if (target == "c") targetPacket = CInteropAdapter::describe("");
        else if (target == "wasm") targetPacket = WasmAdapterV2::describe("");
        else if (target == "x86") targetPacket = X86AdapterV1::describe("");
        else if (target == "arm") targetPacket = ARMAdapterV1::describe("");
        else targetPacket = packet;

        auto layout = MemoryLayoutChecker::assess(packet, targetPacket);
        auto contract = HostBoundaryContract::build(packet);
        LowLevelEntry entry{contract.needsReview, false, layout.compatible ? "aligned" : "layout_mismatch"};
        auto report = LowLevelAcceptanceReportModel::build({entry});

        return {
            {"success", true},
            {"abi", AbiCallingConventionModel::toJson(packet)},
            {"layout", MemoryLayoutChecker::toJson(layout)},
            {"contract", HostBoundaryContract::toJson(contract)},
            {"acceptance", LowLevelAcceptanceReportModel::toJson(report)}
        };
    }
