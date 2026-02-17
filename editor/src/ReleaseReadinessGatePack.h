#pragma once
// Step 589: Release Readiness Gate Pack

#include <string>
#include <vector>

struct ReleaseGateSignals {
    bool uxChecks = false;
    bool runtimeChecks = false;
    bool regressionChecks = false;
    bool telemetryChecks = false;
    bool documentationChecks = false;
};

struct ReleaseReadinessResult {
    bool ready = false;
    std::vector<std::string> blockedGates;
    std::vector<std::string> notes;
};

class ReleaseReadinessGatePack {
public:
    static ReleaseReadinessResult evaluate(const ReleaseGateSignals& s) {
        ReleaseReadinessResult r;
        if (!s.uxChecks) r.blockedGates.push_back("ux");
        if (!s.runtimeChecks) r.blockedGates.push_back("runtime");
        if (!s.regressionChecks) r.blockedGates.push_back("regression");
        if (!s.telemetryChecks) r.blockedGates.push_back("telemetry");
        if (!s.documentationChecks) r.blockedGates.push_back("documentation");

        r.ready = r.blockedGates.empty();
        if (r.ready) {
            r.notes.push_back("release:go");
            r.notes.push_back("release:gates_all_green");
        } else {
            r.notes.push_back("release:no_go");
            for (const auto& gate : r.blockedGates) r.notes.push_back("blocked:" + gate);
        }
        return r;
    }

    static bool canShipHotfix(const ReleaseGateSignals& s) {
        return s.runtimeChecks && s.regressionChecks;
    }

    static int readinessScore(const ReleaseGateSignals& s) {
        int score = 0;
        if (s.uxChecks) score += 20;
        if (s.runtimeChecks) score += 25;
        if (s.regressionChecks) score += 25;
        if (s.telemetryChecks) score += 15;
        if (s.documentationChecks) score += 15;
        return score;
    }
};
