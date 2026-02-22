#pragma once
// Step 803: ARM assembly lowering adapter v1.

#include <string>

#include <nlohmann/json.hpp>

#include "AbiCallingConventionModel.h"

class ARMAdapterV1 {
public:
    static AbiPacket describe(const std::string& source) {
        auto p = AbiCallingConventionModel::describe(source, "arm");
        if (source.find("aapcs") != std::string::npos) p.callingConvention = "aapcs";
        p.memoryLayout = source.find("packed") != std::string::npos ? "packed" : "arm_default";
        return p;
    }

    static nlohmann::json toJson(const AbiPacket& p) {
        return AbiCallingConventionModel::toJson(p);
    }
};
