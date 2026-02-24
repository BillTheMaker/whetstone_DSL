#pragma once
// Step 1150: Reference implementation harness for IR/evidence specs.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct ReferenceImplementationHarnessForIREvidenceSpecs {
    std::string harnessId;
    std::string irSpecVersion;
    std::string evidenceSpecVersion;
    std::vector<std::string> supportedLanguages;
    bool deterministicReplay = false;
    bool ready = false;
};

class ReferenceImplementationHarnessForIREvidenceSpecsFactory {
public:
    static ReferenceImplementationHarnessForIREvidenceSpecs make(const std::string& harnessId,
                                                                 const std::string& irSpecVersion,
                                                                 const std::string& evidenceSpecVersion,
                                                                 const std::vector<std::string>& supportedLanguages,
                                                                 bool deterministicReplay) {
        bool ready = !harnessId.empty() && !irSpecVersion.empty() && !evidenceSpecVersion.empty() &&
                     !supportedLanguages.empty() && deterministicReplay;
        return {harnessId, irSpecVersion, evidenceSpecVersion, supportedLanguages, deterministicReplay, ready};
    }

    static nlohmann::json toJson(const ReferenceImplementationHarnessForIREvidenceSpecs& h) {
        return {{"harness_id", h.harnessId},
                {"ir_spec_version", h.irSpecVersion},
                {"evidence_spec_version", h.evidenceSpecVersion},
                {"supported_languages", h.supportedLanguages},
                {"deterministic_replay", h.deterministicReplay},
                {"ready", h.ready}};
    }
};
