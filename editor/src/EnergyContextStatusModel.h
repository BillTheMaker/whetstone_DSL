#pragma once
// Step 657: Energy context status bar model

#include <string>

struct EnergyContextState {
    std::string mode = "Normal";
    int watts = 0;
    int batteryPercent = 100;
};

class EnergyContextStatusModel {
public:
    static std::string statusLabel(const EnergyContextState& e) {
        if (e.mode == "Conservation") {
            return "Conservation " + std::to_string(e.batteryPercent) + "%";
        }
        return e.mode + " " + std::to_string(e.watts) + "W";
    }

    static bool shouldFlashOrange(const EnergyContextState& e) {
        return e.mode == "Conservation" || e.batteryPercent < 30;
    }

    static std::string promptContextLine(const EnergyContextState& e) {
        return "energyState: " + statusLabel(e);
    }
};
