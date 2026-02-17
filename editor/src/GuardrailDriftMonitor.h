#pragma once
// Step 597: Guardrail Drift Monitor

#include "PolicyGuardrailCatalog.h"

#include <string>
#include <vector>

struct DriftObservation {
    std::string operation;
    std::string observedOutcome;
};

struct DriftFinding {
    std::string operation;
    std::string expectedPolicy;
    std::string observedOutcome;
};

class GuardrailDriftMonitor {
public:
    static std::vector<DriftFinding> detect(const PolicyGuardrailCatalog& catalog,
                                            const std::vector<DriftObservation>& observations) {
        std::vector<DriftFinding> findings;
        for (const auto& o : observations) {
            const auto p = catalog.evaluate(o.operation);
            const std::string expected = decisionName(p.decision);
            if (expected != o.observedOutcome) {
                findings.push_back({o.operation, expected, o.observedOutcome});
            }
        }
        return findings;
    }

    static int driftScore(const std::vector<DriftFinding>& findings, int totalObservations) {
        if (totalObservations <= 0) return 0;
        int score = static_cast<int>((100.0 * findings.size()) / totalObservations);
        if (score > 100) score = 100;
        return score;
    }

    static std::vector<std::string> remediationHints(const std::vector<DriftFinding>& findings) {
        std::vector<std::string> hints;
        for (const auto& f : findings) {
            if (f.expectedPolicy == "deny") hints.push_back("enforce_hard_block:" + f.operation);
            else if (f.expectedPolicy == "require_review") hints.push_back("restore_review_gate:" + f.operation);
            else hints.push_back("align_allow_path:" + f.operation);
        }
        return hints;
    }

private:
    static std::string decisionName(GuardrailDecision d) {
        if (d == GuardrailDecision::Allow) return "allow";
        if (d == GuardrailDecision::Deny) return "deny";
        return "require_review";
    }
};
