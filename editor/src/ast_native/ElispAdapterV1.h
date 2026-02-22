#pragma once
// Step 772: Elisp lowering/raising adapters.

#include <string>

#include <nlohmann/json.hpp>

#include "SExpressionCanonicalLowering.h"

class ElispAdapterV1 {
public:
    static ASTNativeLoweringPacket lower(const std::string& source) {
        auto p = SExpressionCanonicalLowering::lower(source, "elisp");
        p.macroLike = p.macroLike || source.find("defmacro") != std::string::npos;
        return p;
    }

    static ASTNativeRaisingPacket raise(const std::string& ir, const std::string& profile) {
        return SExpressionCanonicalLowering::raise(ir, "elisp", profile == "strict" ? "macro_safe" : "canonical");
    }

    static nlohmann::json toJson(const ASTNativeLoweringPacket& p) {
        return SExpressionCanonicalLowering::toJson(p);
    }

    static nlohmann::json toJson(const ASTNativeRaisingPacket& p) {
        return SExpressionCanonicalLowering::toJson(p);
    }
};
