#pragma once
// Step 546: Confidence Calibration for Constrained Paths

#include <algorithm>
#include <map>
#include <string>

struct CalibrationSample {
    std::string route;
    bool success = false;
    bool postApplyFailure = false;
};

struct CalibratedConfidence {
    double adjustedConfidence = 0.0;
    std::string recommendedRoute;
    std::map<std::string, double> routeScores;
};

class ConstrainedConfidenceCalibrator {
public:
    static CalibratedConfidence calibrate(
        double baseConfidence,
        const std::map<std::string, int>& historicalSuccesses,
        const std::map<std::string, int>& historicalAttempts,
        const std::map<std::string, int>& historicalPostApplyFailures,
        const std::string& preferredRoute) {
        CalibratedConfidence out;

        out.routeScores["deterministic_template"] =
            scoreFor("deterministic_template", baseConfidence,
                     historicalSuccesses, historicalAttempts, historicalPostApplyFailures);
        out.routeScores["constrained_worker"] =
            scoreFor("constrained_worker", baseConfidence,
                     historicalSuccesses, historicalAttempts, historicalPostApplyFailures);
        out.routeScores["general_worker"] =
            scoreFor("general_worker", baseConfidence,
                     historicalSuccesses, historicalAttempts, historicalPostApplyFailures);

        // Mild preference bias only if route is not heavily penalized.
        if (!preferredRoute.empty()) {
            out.routeScores[preferredRoute] += 0.03;
        }
        for (auto& kv : out.routeScores) {
            kv.second = clamp(kv.second);
        }

        out.recommendedRoute = bestRoute(out.routeScores);
        out.adjustedConfidence = clamp(out.routeScores[out.recommendedRoute]);
        return out;
    }

private:
    static double scoreFor(const std::string& route,
                           double base,
                           const std::map<std::string, int>& successes,
                           const std::map<std::string, int>& attempts,
                           const std::map<std::string, int>& postApplyFailures) {
        const int s = lookup(successes, route);
        const int a = lookup(attempts, route);
        const int f = lookup(postApplyFailures, route);

        double successRate = (a > 0) ? (double)s / (double)a : 0.5;
        double failurePenalty = (a > 0) ? (double)f / (double)a : 0.0;

        double score = base;
        score += 0.35 * (successRate - 0.5);
        score -= 0.45 * failurePenalty;

        // Repeated post-apply failures trigger stronger decay.
        if (f >= 3) score -= 0.10;
        if (f >= 5) score -= 0.10;

        return clamp(score);
    }

    static int lookup(const std::map<std::string, int>& m, const std::string& key) {
        auto it = m.find(key);
        return it == m.end() ? 0 : it->second;
    }

    static std::string bestRoute(const std::map<std::string, double>& scores) {
        std::string best = "general_worker";
        double bestScore = -1.0;
        for (const auto& kv : scores) {
            if (kv.second > bestScore) {
                bestScore = kv.second;
                best = kv.first;
            }
        }
        return best;
    }

    static double clamp(double x) {
        return std::max(0.0, std::min(1.0, x));
    }
};
