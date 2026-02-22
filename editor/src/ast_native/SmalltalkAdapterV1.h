#pragma once
// Step 773: Smalltalk lowering/raising adapters.

#include <string>

#include <nlohmann/json.hpp>

#include "SExpressionCanonicalLowering.h"

class SmalltalkAdapterV1 {
public:
    static ASTNativeLoweringPacket lower(const std::string& source) {
        auto p = SExpressionCanonicalLowering::lower(source, "smalltalk");
        p.messageSendLike = true;
        p.listDepth = source.empty() ? 0 : 1;
        return p;
    }

    static ASTNativeRaisingPacket raise(const std::string& ir, const std::string& profile) {
        return SExpressionCanonicalLowering::raise(ir, "smalltalk", profile == "strict" ? "message_safe" : "message_friendly");
    }

    static nlohmann::json toJson(const ASTNativeLoweringPacket& p) {
        return SExpressionCanonicalLowering::toJson(p);
    }

    static nlohmann::json toJson(const ASTNativeRaisingPacket& p) {
        return SExpressionCanonicalLowering::toJson(p);
    }
};
