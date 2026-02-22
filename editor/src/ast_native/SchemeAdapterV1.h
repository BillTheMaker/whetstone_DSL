#pragma once
// Step 771: Scheme lowering/raising adapters.

#include <string>

#include <nlohmann/json.hpp>

#include "SExpressionCanonicalLowering.h"

class SchemeAdapterV1 {
public:
    static ASTNativeLoweringPacket lower(const std::string& source) {
        return SExpressionCanonicalLowering::lower(source, "scheme");
    }

    static ASTNativeRaisingPacket raise(const std::string& ir, const std::string& profile) {
        return SExpressionCanonicalLowering::raise(ir, "scheme", profile == "strict" ? "hygienic" : "canonical");
    }

    static nlohmann::json toJson(const ASTNativeLoweringPacket& p) {
        return SExpressionCanonicalLowering::toJson(p);
    }

    static nlohmann::json toJson(const ASTNativeRaisingPacket& p) {
        return SExpressionCanonicalLowering::toJson(p);
    }
};
