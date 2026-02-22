#pragma once
// Step 759: Kotlin lowering/raising adapters.

#include <string>

#include <nlohmann/json.hpp>

#include "ManagedPacketTypes.h"

class KotlinAdapterV1 {
public:
    static ManagedLoweringPacket lower(const std::string& source) {
        ManagedLoweringPacket p;
        p.sourceLanguage = "kotlin";
        p.irSummary = source.empty() ? "empty_kotlin_unit" : "kotlin_ir_v1";
        p.hasNullableSyntax = source.find('?') != std::string::npos;
        p.hasOptionalType = source.find("Option") != std::string::npos;
        p.asyncSignalCount = source.find("suspend") != std::string::npos ? 1 : 0;
        p.adtLike = source.find("sealed class") != std::string::npos || source.find("when") != std::string::npos;
        return p;
    }

    static ManagedRaisingPacket raise(const std::string& ir, const std::string& profile) {
        ManagedRaisingPacket p;
        p.targetLanguage = "kotlin";
        p.codePreview = "// kotlin raised from " + ir;
        p.nullabilityModel = profile == "strict" ? "kotlin_nullable_strict" : "kotlin_nullable_balanced";
        p.asyncModel = "coroutines";
        return p;
    }

    static nlohmann::json toJson(const ManagedLoweringPacket& p) {
        return {
            {"source_language", p.sourceLanguage},
            {"ir_summary", p.irSummary},
            {"has_nullable_syntax", p.hasNullableSyntax},
            {"has_optional_type", p.hasOptionalType},
            {"async_signal_count", p.asyncSignalCount},
            {"adt_like", p.adtLike}
        };
    }

    static nlohmann::json toJson(const ManagedRaisingPacket& p) {
        return {
            {"target_language", p.targetLanguage},
            {"code_preview", p.codePreview},
            {"nullability_model", p.nullabilityModel},
            {"async_model", p.asyncModel}
        };
    }
};
