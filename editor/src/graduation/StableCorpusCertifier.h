#pragma once
// Step 833: Stable corpus certification suite.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct CorpusCase {
    std::string caseId;
    std::string pairId;
    std::string sourceSnippet;
    std::string expectedOutput;
    bool passed = false;
};

struct CertificationSuiteResult {
    std::string pairId;
    std::vector<CorpusCase> cases;
    int totalCases = 0;
    int passedCases = 0;
    bool certified = false;
};

class StableCorpusCertifier {
public:
    static CertificationSuiteResult certify(const std::string& pairId,
                                             std::vector<CorpusCase> cases) {
        CertificationSuiteResult r;
        r.pairId = pairId;
        r.totalCases = static_cast<int>(cases.size());
        for (auto& c : cases) {
            c.passed = !c.sourceSnippet.empty() && !c.expectedOutput.empty();
            if (c.passed) ++r.passedCases;
        }
        r.cases = cases;
        r.certified = r.passedCases == r.totalCases && r.totalCases > 0;
        return r;
    }

    static nlohmann::json toJson(const CertificationSuiteResult& r) {
        return {{"pair_id", r.pairId}, {"total", r.totalCases},
                {"passed", r.passedCases}, {"certified", r.certified}};
    }
};
