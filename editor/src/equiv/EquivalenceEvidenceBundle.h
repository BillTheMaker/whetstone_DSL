#pragma once
// Step 727: equivalence evidence bundle exporter.

#include <nlohmann/json.hpp>

#include "DifferentialExecutionHarness.h"
#include "PropertyEquivalenceRunner.h"
#include "FuzzDifferentialRunner.h"

struct EquivalenceEvidenceBundle {
    DifferentialRunResult differential;
    PropertyCheckResult property;
    std::vector<FuzzCase> fuzz;
};

class EquivalenceEvidenceBundleModel {
public:
    static EquivalenceEvidenceBundle build(const DifferentialRunResult& differential,
                                           const PropertyCheckResult& property,
                                           const std::vector<FuzzCase>& fuzz) {
        return {differential, property, fuzz};
    }

    static nlohmann::json toJson(const EquivalenceEvidenceBundle& b) {
        return {
            {"differential", DifferentialExecutionHarness::toJson(b.differential)},
            {"property", PropertyEquivalenceRunner::toJson(b.property)},
            {"fuzz", FuzzDifferentialRunner::toJson(b.fuzz)}
        };
    }
};
