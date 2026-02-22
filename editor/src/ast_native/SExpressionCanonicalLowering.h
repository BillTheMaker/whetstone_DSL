#pragma once
// Step 769: S-expression canonical lowering layer.

#include <string>

#include <nlohmann/json.hpp>

struct ASTNativeLoweringPacket {
    std::string sourceLanguage;
    std::string canonicalForm;
    int listDepth = 0;
    bool messageSendLike = false;
    bool macroLike = false;
};

struct ASTNativeRaisingPacket {
    std::string targetLanguage;
    std::string codePreview;
    std::string projectionStyle;
};

class SExpressionCanonicalLowering {
public:
    static ASTNativeLoweringPacket lower(const std::string& source, const std::string& sourceLanguage) {
        ASTNativeLoweringPacket p;
        p.sourceLanguage = sourceLanguage;
        p.canonicalForm = source.empty() ? "empty_form" : "sexpr_ir_v1";
        p.listDepth = source.find("(") != std::string::npos ? 1 : 0;
        p.messageSendLike = source.find(" ") != std::string::npos && source.find(":") != std::string::npos;
        p.macroLike = source.find("macro") != std::string::npos || source.find("defmacro") != std::string::npos;
        return p;
    }

    static ASTNativeRaisingPacket raise(const std::string& ir,
                                        const std::string& targetLanguage,
                                        const std::string& style) {
        ASTNativeRaisingPacket p;
        p.targetLanguage = targetLanguage;
        p.codePreview = ";; raised " + targetLanguage + " from " + ir;
        p.projectionStyle = style.empty() ? "canonical" : style;
        return p;
    }

    static nlohmann::json toJson(const ASTNativeLoweringPacket& p) {
        return {
            {"source_language", p.sourceLanguage},
            {"canonical_form", p.canonicalForm},
            {"list_depth", p.listDepth},
            {"message_send_like", p.messageSendLike},
            {"macro_like", p.macroLike}
        };
    }

    static nlohmann::json toJson(const ASTNativeRaisingPacket& p) {
        return {
            {"target_language", p.targetLanguage},
            {"code_preview", p.codePreview},
            {"projection_style", p.projectionStyle}
        };
    }
};
