#pragma once
// Step 730: sanitizer gate integration.

#include <string>

#include <nlohmann/json.hpp>

struct SanitizerGateProfile {
    bool requireAsan = true;
    bool requireUbsan = true;
};

struct SanitizerGateResult {
    bool asanClean = false;
    bool ubsanClean = false;
    bool pass = false;
};

class SanitizerGateIntegration {
public:
    static SanitizerGateResult evaluate(const SanitizerGateProfile& profile,
                                        bool asanClean,
                                        bool ubsanClean) {
        SanitizerGateResult r;
        r.asanClean = asanClean;
        r.ubsanClean = ubsanClean;
        r.pass = (!profile.requireAsan || asanClean) && (!profile.requireUbsan || ubsanClean);
        return r;
    }

    static nlohmann::json toJson(const SanitizerGateResult& r) {
        return {{"asan_clean", r.asanClean}, {"ubsan_clean", r.ubsanClean}, {"pass", r.pass}};
    }
};
