#pragma once
// Step 578: Phase 32a Integration

#include "ArchitectReviewSurface.h"
#include "MarkdownSpecParser.h"
#include "RequirementNormalizationConflictDetector.h"
#include "ScopeMilestoneDecomposer.h"

#include <string>
#include <vector>

struct Phase32aInput {
    std::string markdown;
    std::string reviewer = "architect";
    bool autoApproveConstraints = true;
    bool forceRejectFirstConstraint = false;
};

struct Phase32aResult {
    bool parseReady = false;
    bool normalizationReady = false;
    bool decompositionReady = false;
    bool reviewReady = false;
    bool phase32aPass = false;
    std::size_t parsedSectionCount = 0;
    std::size_t conflictCount = 0;
    int overallUncertainty = 0;
    ConstraintReviewSummary reviewSummary;
    std::vector<std::string> notes;
};

class Phase32aIntegration {
public:
    static Phase32aResult run(const Phase32aInput& input) {
        Phase32aResult result;
        ParsedMarkdownSpec parsed;
        std::string error;
        if (!MarkdownSpecParser::parse(input.markdown, &parsed, &error)) {
            result.notes.push_back("fail:markdown_parse");
            return finalize(result);
        }
        result.parseReady = true;
        result.parsedSectionCount = parsed.sections.size();

        RequirementNormalizationResult normalized;
        if (!RequirementNormalizationConflictDetector::normalize(parsed, &normalized, &error)) {
            result.notes.push_back("fail:normalization");
            return finalize(result);
        }
        result.normalizationReady = true;
        result.conflictCount = normalized.conflicts.size();

        DecomposedScopePlan plan;
        if (!ScopeMilestoneDecomposer::decompose(normalized, &plan, &error)) {
            result.notes.push_back("fail:decomposition");
            return finalize(result);
        }
        result.decompositionReady = true;
        result.overallUncertainty = plan.overallUncertainty;

        ArchitectReviewSurface review;
        if (!review.loadConstraints(normalized.requirements, &error)) {
            result.notes.push_back("fail:review_load");
            return finalize(result);
        }

        const auto items = review.items();
        for (std::size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            bool ok = false;
            if (input.forceRejectFirstConstraint && i == 0) {
                ok = review.reject(item.requirementId, input.reviewer, &error);
            } else if (input.autoApproveConstraints) {
                ok = review.approve(item.requirementId, input.reviewer, &error);
            } else {
                ok = review.modify(item.requirementId, item.originalText + " reviewed", input.reviewer, &error);
            }
            if (!ok) {
                result.notes.push_back("fail:review_decision");
                return finalize(result);
            }
        }

        result.reviewSummary = review.summarize();
        result.reviewReady = result.reviewSummary.allDecided;
        if (!result.reviewReady) {
            result.notes.push_back("fail:review_incomplete");
            return finalize(result);
        }

        result.notes.push_back("phase32a:markdown_intake_cycle_ready");
        return finalize(result);
    }

private:
    static Phase32aResult finalize(Phase32aResult result) {
        result.phase32aPass = result.parseReady &&
                             result.normalizationReady &&
                             result.decompositionReady &&
                             result.reviewReady;
        if (!result.phase32aPass) result.notes.push_back("phase32a:blocked");
        return result;
    }
};
