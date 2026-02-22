#pragma once
// Step 770: Lisp lowering/raising adapters.

#include <string>

#include <nlohmann/json.hpp>

#include "SExpressionCanonicalLowering.h"

class LispAdapterV1 {
public:
    static ASTNativeLoweringPacket lower(const std::string& source) {
        return SExpressionCanonicalLowering::lower(source, "lisp");
    }

    static ASTNativeRaisingPacket raise(const std::string& ir, const std::string& profile) {
        return SExpressionCanonicalLowering::raise(ir, "lisp", profile == "strict" ? "hygienic" : "canonical");
    }

    static nlohmann::json toJson(const ASTNativeLoweringPacket& p) {
        return SExpressionCanonicalLowering::toJson(p);
    }

    static nlohmann::json toJson(const ASTNativeRaisingPacket& p) {
        return SExpressionCanonicalLowering::toJson(p);
    }
};
