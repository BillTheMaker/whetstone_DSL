#pragma once
// Step 863: A/B harness for hint effectiveness.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct ABVariant {
    std::string variantId; // "control", "hint_guided"
    int reviewsTotal = 0;
    int reviewsAccepted = 0;
    float acceptanceRate = 0.0f;
};

struct ABHarnessResult {
    std::string experimentId;
    ABVariant control;
    ABVariant treatment;
    bool treatmentWins = false;
    float lift = 0.0f;
};

class HintABHarness {
public:
    static ABHarnessResult evaluate(const std::string& experimentId,
                                     const ABVariant& control,
                                     const ABVariant& treatment) {
        ABHarnessResult r;
        r.experimentId = experimentId;
        r.control = control;
        r.treatment = treatment;
        float ctrlRate = control.reviewsTotal > 0
            ? static_cast<float>(control.reviewsAccepted) / control.reviewsTotal : 0.0f;
        float trtRate = treatment.reviewsTotal > 0
            ? static_cast<float>(treatment.reviewsAccepted) / treatment.reviewsTotal : 0.0f;
        r.lift = trtRate - ctrlRate;
        r.treatmentWins = r.lift > 0.05f;
        return r;
    }

    static nlohmann::json toJson(const ABHarnessResult& r) {
        return {{"experiment_id", r.experimentId}, {"lift", r.lift},
                {"treatment_wins", r.treatmentWins},
                {"control_acceptance", r.control.reviewsTotal > 0
                    ? static_cast<float>(r.control.reviewsAccepted)/r.control.reviewsTotal : 0.0f},
                {"treatment_acceptance", r.treatment.reviewsTotal > 0
                    ? static_cast<float>(r.treatment.reviewsAccepted)/r.treatment.reviewsTotal : 0.0f}};
    }
};
