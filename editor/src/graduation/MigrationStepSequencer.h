#pragma once
// Step 902: Migration step sequencer — produces an ordered list of migration steps.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct MigrationStep {
    std::string stepId;
    std::string description;
    std::string prerequisite;  // empty string means no prerequisite
    bool required = true;
};

class MigrationStepSequencer {
public:
    // Generate a canonical step sequence for migrating srcRuntime -> tgtRuntime.
    // Always includes at minimum: analyze_source, adapt_semantics, verify_target.
    static std::vector<MigrationStep> sequence(const std::string& pairId,
                                               const std::string& srcRuntime,
                                               const std::string& tgtRuntime) {
        std::vector<MigrationStep> steps;

        steps.push_back({
            pairId + ":analyze_source",
            "Analyze source runtime (" + srcRuntime + ") semantics and dependencies",
            "",
            true
        });
        steps.push_back({
            pairId + ":adapt_semantics",
            "Adapt semantic constructs from " + srcRuntime + " to " + tgtRuntime,
            pairId + ":analyze_source",
            true
        });
        steps.push_back({
            pairId + ":verify_target",
            "Verify adapted code against target runtime (" + tgtRuntime + ") constraints",
            pairId + ":adapt_semantics",
            true
        });

        // Optional enrichment steps when both runtimes are non-empty.
        if (!srcRuntime.empty() && !tgtRuntime.empty()) {
            steps.push_back({
                pairId + ":run_equivalence_check",
                "Run equivalence checks to validate behavioral parity post-migration",
                pairId + ":verify_target",
                false
            });
            steps.push_back({
                pairId + ":generate_migration_report",
                "Generate migration report summarising findings and any residual risks",
                pairId + ":run_equivalence_check",
                false
            });
        }

        return steps;
    }

    static nlohmann::json toJson(const MigrationStep& s) {
        return {
            {"step_id",      s.stepId},
            {"description",  s.description},
            {"prerequisite", s.prerequisite},
            {"required",     s.required}
        };
    }
};
