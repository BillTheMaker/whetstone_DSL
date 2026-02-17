#pragma once
// Step 603: Compliance Operational Readiness

#include <algorithm>
#include <string>
#include <vector>

struct ComplianceReadinessSnapshot {
    int controlsCovered = 0;
    int artifactCount = 0;
    int signedAttestations = 0;
    int expiringSoon = 0;
    int approvedExceptions = 0;
    int openRunbooks = 0;
};

class ComplianceOperationalReadiness {
public:
    static bool validateInput(const ComplianceReadinessSnapshot& s, std::string* error) {
        if (!error) return false;
        error->clear();
        if (s.controlsCovered < 0) return fail(error, "controls_covered_invalid");
        if (s.artifactCount < 0) return fail(error, "artifact_count_invalid");
        if (s.signedAttestations < 0) return fail(error, "signed_attestations_invalid");
        if (s.expiringSoon < 0) return fail(error, "expiring_soon_invalid");
        if (s.approvedExceptions < 0) return fail(error, "approved_exceptions_invalid");
        if (s.openRunbooks < 0) return fail(error, "open_runbooks_invalid");
        return true;
    }

    static int readinessScore(const ComplianceReadinessSnapshot& s) {
        int score = s.controlsCovered * 5 +
                    s.artifactCount * 2 +
                    s.signedAttestations * 4 +
                    s.approvedExceptions * 2 -
                    s.expiringSoon * 3 -
                    s.openRunbooks * 4;
        return std::max(0, std::min(100, score));
    }

    static std::vector<std::string> blockingFindings(const ComplianceReadinessSnapshot& s) {
        std::vector<std::string> findings;
        if (s.controlsCovered == 0) findings.push_back("controls_uncovered");
        if (s.artifactCount == 0) findings.push_back("artifacts_missing");
        if (s.signedAttestations == 0) findings.push_back("attestations_missing");
        if (s.expiringSoon > 0) findings.push_back("attestations_expiring");
        if (s.openRunbooks > 0) findings.push_back("incidents_open");
        return findings;
    }

    static bool releaseEligible(const ComplianceReadinessSnapshot& s) {
        return blockingFindings(s).empty();
    }

private:
    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }
};
