#pragma once
// Step 762: VB.NET lowering/raising adapters.

#include <string>

#include <nlohmann/json.hpp>

#include "ManagedPacketTypes.h"

class VbNetAdapterV1 {
public:
    static ManagedLoweringPacket lower(const std::string& source) {
        ManagedLoweringPacket p;
        p.sourceLanguage = "vbnet";
        p.irSummary = source.empty() ? "empty_vbnet_unit" : "vbnet_ir_v1";
        p.hasNullableSyntax = source.find("?") != std::string::npos || source.find("Nothing") != std::string::npos;
        p.hasOptionalType = source.find("Nullable") != std::string::npos;
        p.asyncSignalCount = (source.find("Async") != std::string::npos || source.find("Task") != std::string::npos) ? 1 : 0;
        p.adtLike = source.find("Select Case") != std::string::npos;
        return p;
    }

    static ManagedRaisingPacket raise(const std::string& ir, const std::string& profile) {
        ManagedRaisingPacket p;
        p.targetLanguage = "vbnet";
        p.codePreview = "' vbnet raised from " + ir;
        p.nullabilityModel = profile == "strict" ? "vbnet_nullable_strict" : "vbnet_nullable_bridge";
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
