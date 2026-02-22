#pragma once
// Step 814: Confidence calibration for recovered intent.

#include <string>

#include <nlohmann/json.hpp>

class ConfidenceCalibration {
public:
    static double calibrate(double raw) {
        if (raw > 0.8) return 0.95;
        if (raw > 0.5) return raw + 0.1;
        return raw;
    }

    static nlohmann::json toJson(double calibrated) {
        return {{"calibrated_confidence", calibrated}};
    }
};
