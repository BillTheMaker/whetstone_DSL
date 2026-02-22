#pragma once
// Step 744: Java raising adapter v1.

#include <nlohmann/json.hpp>

#include "CRaisingAdapterV1.h"

class JavaRaisingAdapterV1 {
public:
    static RaisingPacket raise(const std::string& irSummary, const std::string& profile) {
        return {"java", "// java raise " + irSummary, profile};
    }

    static nlohmann::json toJson(const RaisingPacket& p) {
        return {{"target_language", p.targetLanguage}, {"code_preview", p.codePreview}, {"profile", p.profile}};
    }
};
