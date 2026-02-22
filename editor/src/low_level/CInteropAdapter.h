#pragma once
// Step 800: C interop adapter deepening.

#include <string>

#include <nlohmann/json.hpp>

#include "AbiCallingConventionModel.h"

class CInteropAdapter {
public:
    static AbiPacket describe(const std::string& source) {
        auto packet = AbiCallingConventionModel::describe(source, "c");
        if (source.find("__stdcall") != std::string::npos) packet.callingConvention = "stdcall";
        return packet;
    }

    static nlohmann::json toJson(const AbiPacket& packet) {
        return AbiCallingConventionModel::toJson(packet);
    }
};
