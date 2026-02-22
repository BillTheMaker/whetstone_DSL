#pragma once
// Step 801: WASM lowering/raising adapters v2.

#include <string>

#include <nlohmann/json.hpp>

#include "AbiCallingConventionModel.h"

class WasmAdapterV2 {
public:
    static AbiPacket describe(const std::string& source) {
        auto packet = AbiCallingConventionModel::describe(source, "wasm");
        packet.callingConvention = "wasmabi";
        packet.memoryLayout = "linear";
        packet.hostBoundary = source.find("host") != std::string::npos;
        return packet;
    }

    static nlohmann::json toJson(const AbiPacket& packet) {
        return AbiCallingConventionModel::toJson(packet);
    }
};
