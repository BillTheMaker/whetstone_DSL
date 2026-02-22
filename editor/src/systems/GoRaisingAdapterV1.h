#pragma once
// Step 743: Go raising adapter v1.

#include <nlohmann/json.hpp>

#include "CRaisingAdapterV1.h"

class GoRaisingAdapterV1 {
public:
    static RaisingPacket raise(const std::string& irSummary, const std::string& profile) {
        return {"go", "// go raise " + irSummary, profile};
    }

    static nlohmann::json toJson(const RaisingPacket& p) {
        return {{"target_language", p.targetLanguage}, {"code_preview", p.codePreview}, {"profile", p.profile}};
    }
};
