#pragma once
// Step 761: F# lowering/raising adapters.

#include <string>

#include <nlohmann/json.hpp>

#include "ManagedPacketTypes.h"

class FSharpAdapterV1 {
public:
    static ManagedLoweringPacket lower(const std::string& source) {
        ManagedLoweringPacket p;
        p.sourceLanguage = "fsharp";
        p.irSummary = source.empty() ? "empty_fsharp_unit" : "fsharp_ir_v1";
        p.hasNullableSyntax = source.find("Nullable") != std::string::npos;
        p.hasOptionalType = source.find("option") != std::string::npos;
        p.asyncSignalCount = (source.find("async") != std::string::npos || source.find("task") != std::string::npos) ? 1 : 0;
        p.adtLike = source.find("type") != std::string::npos && source.find("|") != std::string::npos;
        return p;
    }

    static ManagedRaisingPacket raise(const std::string& ir, const std::string& profile) {
        ManagedRaisingPacket p;
        p.targetLanguage = "fsharp";
        p.codePreview = "// fsharp raised from " + ir;
        p.nullabilityModel = profile == "strict" ? "fsharp_option_first" : "fsharp_nullable_bridge";
        p.asyncModel = "async_workflow";
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
