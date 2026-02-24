#pragma once
// Step 1652: C++ text-first default profile model.
#include <string>
#include <nlohmann/json.hpp>

struct CppTextFirstDefaultProfile {
    std::string profileId;
    std::string language;
    std::string defaultMode;
    bool deterministicProjection = true;
    bool valid = false;
};

class CppTextFirstDefaultProfileFactory {
public:
    static CppTextFirstDefaultProfile make(const std::string& profileId,
                                           const std::string& language,
                                           const std::string& defaultMode,
                                           bool deterministicProjection) {
        bool valid = !profileId.empty()
            && language == "cpp"
            && defaultMode == "text_first";
        return {profileId, language, defaultMode, deterministicProjection, valid};
    }

    static nlohmann::json toJson(const CppTextFirstDefaultProfile& v) {
        return {{"profile_id", v.profileId},
                {"language", v.language},
                {"default_mode", v.defaultMode},
                {"deterministic_projection", v.deterministicProjection},
                {"valid", v.valid}};
    }
};
