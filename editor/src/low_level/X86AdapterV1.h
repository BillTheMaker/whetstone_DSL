#pragma once
// Step 802: x86 assembly lowering adapter v1.

#include <string>

#include <nlohmann/json.hpp>

#include "AbiCallingConventionModel.h"

class X86AdapterV1 {
public:
    static AbiPacket describe(const std::string& source) {
        auto p = AbiCallingConventionModel::describe(source, "x86");
        if (source.find("fastcall") != std::string::npos) p.callingConvention = "fastcall";
        if (source.find("orphan") != std::string::npos) p.hostBoundary = true;
        return p;
    }

    static nlohmann::json toJson(const AbiPacket& p) {
        return AbiCallingConventionModel::toJson(p);
    }
};
