#pragma once
// Step 713: error model mapping policy.

#include <string>

#include <nlohmann/json.hpp>

#include "SemanticCoreIR.h"

struct CppErrorModelDecision {
    std::string profile;
    std::string resultType; // expected, status_or, exceptions
    bool panicTranslatedToTerminate = false;
};

class CppErrorModelMapping {
public:
    static CppErrorModelDecision map(const SemanticCoreIR& ir,
                                     const std::string& profile = "safe-first") {
        CppErrorModelDecision d;
        d.profile = profile;
        if (profile == "interop-first") d.resultType = "status_or";
        else if (profile == "perf-first") d.resultType = "expected";
        else d.resultType = "expected";

        const std::string dump = ir.contracts.dump();
        d.panicTranslatedToTerminate = dump.find("panic") != std::string::npos;
        return d;
    }

    static nlohmann::json toJson(const CppErrorModelDecision& d) {
        return {
            {"profile", d.profile},
            {"resultType", d.resultType},
            {"panicTranslatedToTerminate", d.panicTranslatedToTerminate}
        };
    }
};
