#pragma once
// Step 742: C raising adapter v1.

#include <string>

#include <nlohmann/json.hpp>

struct RaisingPacket {
    std::string targetLanguage;
    std::string codePreview;
    std::string profile;
};

class CRaisingAdapterV1 {
public:
    static RaisingPacket raise(const std::string& irSummary, const std::string& profile) {
        return {"c", "/* c raise */ " + irSummary, profile};
    }

    static nlohmann::json toJson(const RaisingPacket& p) {
        return {{"target_language", p.targetLanguage}, {"code_preview", p.codePreview}, {"profile", p.profile}};
    }
};
