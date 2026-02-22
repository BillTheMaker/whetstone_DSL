#pragma once
// Step 760: C# lowering/raising adapters.

#include <string>

#include <nlohmann/json.hpp>

#include "ManagedPacketTypes.h"

class CSharpAdapterV1 {
public:
    static ManagedLoweringPacket lower(const std::string& source) {
        ManagedLoweringPacket p;
        p.sourceLanguage = "csharp";
        p.irSummary = source.empty() ? "empty_csharp_unit" : "csharp_ir_v1";
        p.hasNullableSyntax = source.find('?') != std::string::npos;
        p.hasOptionalType = source.find("Option") != std::string::npos;
        p.asyncSignalCount = (source.find("async") != std::string::npos || source.find("Task") != std::string::npos) ? 1 : 0;
        p.adtLike = source.find("record") != std::string::npos || source.find("switch") != std::string::npos;
        return p;
    }

    static ManagedRaisingPacket raise(const std::string& ir, const std::string& profile) {
        ManagedRaisingPacket p;
        p.targetLanguage = "csharp";
        p.codePreview = "// csharp raised from " + ir;
        p.nullabilityModel = profile == "strict" ? "csharp_nrt_enabled" : "csharp_nrt_mixed";
        p.asyncModel = "task";
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
