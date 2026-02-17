#pragma once
// Step 582: Intake-to-Queue Simulation Harness

#include "AcceptanceCriteriaBinding.h"
#include "ArchitectReviewSurface.h"
#include "MarkdownSpecParser.h"
#include "RequirementNormalizationConflictDetector.h"
#include "ScopeMilestoneDecomposer.h"
#include "TaskitemConfidenceAmbiguity.h"
#include "TaskitemGeneratorV2.h"

#include <string>
#include <vector>

struct IntakeQueueSimulationInput {
    std::string markdown;
    std::string reviewer = "architect";
    bool modifyConstraints = false;
};

struct IntakeQueueSimulationResult {
    bool ok = false;
    std::size_t queuedCount = 0;
    std::size_t escalatedCount = 0;
    std::size_t boundCheckCount = 0;
    std::vector<BoundTaskitem> queue;
    std::vector<std::string> notes;
};

class IntakeToQueueSimulationHarness {
public:
    static IntakeQueueSimulationResult run(const IntakeQueueSimulationInput& input) {
        IntakeQueueSimulationResult result;
        ParsedMarkdownSpec parsed;
        std::string error;
        if (!MarkdownSpecParser::parse(input.markdown, &parsed, &error)) {
            result.notes.push_back("fail:parse");
            return result;
        }

        RequirementNormalizationResult normalized;
        if (!RequirementNormalizationConflictDetector::normalize(parsed, &normalized, &error)) {
            result.notes.push_back("fail:normalize");
            return result;
        }

        DecomposedScopePlan plan;
        if (!ScopeMilestoneDecomposer::decompose(normalized, &plan, &error)) {
            result.notes.push_back("fail:decompose");
            return result;
        }

        ArchitectReviewSurface review;
        if (!review.loadConstraints(normalized.requirements, &error)) {
            result.notes.push_back("fail:review_load");
            return result;
        }
        for (const auto& item : review.items()) {
            const bool ok = input.modifyConstraints
                            ? review.modify(item.requirementId, item.originalText + " reviewed", input.reviewer, &error)
                            : review.approve(item.requirementId, input.reviewer, &error);
            if (!ok) {
                result.notes.push_back("fail:review_decision");
                return result;
            }
        }

        std::vector<GeneratedTaskitem> generated;
        if (!TaskitemGeneratorV2::generate(plan, &generated, &error)) {
            result.notes.push_back("fail:generate");
            return result;
        }

        std::vector<AnnotatedTaskitem> annotated;
        if (!TaskitemConfidenceAmbiguity::annotate(generated, normalized, &annotated, &error)) {
            result.notes.push_back("fail:annotate");
            return result;
        }

        std::vector<BoundTaskitem> bound;
        if (!AcceptanceCriteriaBinding::bind(annotated, normalized.requirements, &bound, &error)) {
            result.notes.push_back("fail:bind_acceptance");
            return result;
        }

        result.queue = bound;
        for (const auto& task : bound) {
            if (task.task.base.queueReady && task.hasCoverage) ++result.queuedCount;
            if (task.task.escalate) ++result.escalatedCount;
            result.boundCheckCount += task.checks.size();
        }
        result.ok = !result.queue.empty();
        if (result.ok) result.notes.push_back("queue:ready");
        return result;
    }
};
