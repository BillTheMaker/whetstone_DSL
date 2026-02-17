#pragma once
// Step 576: Scope and Milestone Decomposer

#include "RequirementNormalizationConflictDetector.h"

#include <algorithm>
#include <string>
#include <vector>

struct DecomposedWorkstream {
    std::string workstreamId;
    std::string title;
    std::vector<std::string> requirementIds;
    int uncertaintyScore = 0;  // 0..100
};

struct DecomposedMilestone {
    std::string milestoneId;
    std::string title;
    std::vector<DecomposedWorkstream> workstreams;
    int uncertaintyScore = 0;  // 0..100
};

struct DecomposedScopePlan {
    std::vector<DecomposedMilestone> milestones;
    int overallUncertainty = 0;  // 0..100
};

class ScopeMilestoneDecomposer {
public:
    static bool decompose(const RequirementNormalizationResult& normalized,
                          DecomposedScopePlan* outPlan,
                          std::string* error) {
        if (!outPlan || !error) return false;
        error->clear();
        if (normalized.requirements.empty()) {
            *error = "requirements_empty";
            return false;
        }

        DecomposedScopePlan plan;
        plan.milestones.push_back(buildMilestone("milestone-1", "Intake Foundation", normalized, {
            NormalizedRequirementKind::Goal,
            NormalizedRequirementKind::Constraint
        }));
        plan.milestones.push_back(buildMilestone("milestone-2", "Execution Readiness", normalized, {
            NormalizedRequirementKind::Dependency,
            NormalizedRequirementKind::Acceptance
        }));

        pruneEmptyMilestones(&plan);
        if (plan.milestones.empty()) {
            *error = "decomposition_empty";
            return false;
        }

        plan.overallUncertainty = computeOverallUncertainty(plan.milestones);
        *outPlan = plan;
        return true;
    }

private:
    static DecomposedMilestone buildMilestone(const std::string& milestoneId,
                                              const std::string& title,
                                              const RequirementNormalizationResult& normalized,
                                              const std::vector<NormalizedRequirementKind>& kinds) {
        DecomposedMilestone milestone;
        milestone.milestoneId = milestoneId;
        milestone.title = title;

        std::vector<NormalizedRequirement> selected;
        for (const auto& requirement : normalized.requirements) {
            if (containsKind(kinds, requirement.kind)) selected.push_back(requirement);
        }

        if (!selected.empty()) {
            DecomposedWorkstream primary;
            primary.workstreamId = milestoneId + "-ws-1";
            primary.title = title + " Primary";
            for (const auto& requirement : selected) primary.requirementIds.push_back(requirement.requirementId);
            primary.uncertaintyScore = computeUncertainty(selected);
            milestone.workstreams.push_back(primary);

            DecomposedWorkstream review;
            review.workstreamId = milestoneId + "-ws-2";
            review.title = "Architect Review";
            for (const auto& conflict : normalized.conflicts) {
                review.requirementIds.push_back(conflict.leftRequirementId);
                review.requirementIds.push_back(conflict.rightRequirementId);
            }
            dedupeIds(&review.requirementIds);
            if (!review.requirementIds.empty()) {
                review.uncertaintyScore = std::min(100, primary.uncertaintyScore + 20);
                milestone.workstreams.push_back(review);
            }
        }

        milestone.uncertaintyScore = computeMilestoneUncertainty(milestone.workstreams);
        return milestone;
    }

    static bool containsKind(const std::vector<NormalizedRequirementKind>& kinds,
                             NormalizedRequirementKind kind) {
        for (auto candidate : kinds) {
            if (candidate == kind) return true;
        }
        return false;
    }

    static int computeUncertainty(const std::vector<NormalizedRequirement>& requirements) {
        int score = 15;
        for (const auto& requirement : requirements) {
            if (requirement.ambiguous) score += 15;
            if (requirement.kind == NormalizedRequirementKind::Constraint) score += 5;
            if (requirement.kind == NormalizedRequirementKind::Acceptance) score -= 2;
        }
        if (requirements.size() > 6) score += 10;
        return std::max(0, std::min(100, score));
    }

    static int computeMilestoneUncertainty(const std::vector<DecomposedWorkstream>& workstreams) {
        if (workstreams.empty()) return 0;
        int sum = 0;
        for (const auto& workstream : workstreams) sum += workstream.uncertaintyScore;
        return sum / static_cast<int>(workstreams.size());
    }

    static int computeOverallUncertainty(const std::vector<DecomposedMilestone>& milestones) {
        if (milestones.empty()) return 0;
        int sum = 0;
        for (const auto& milestone : milestones) sum += milestone.uncertaintyScore;
        return sum / static_cast<int>(milestones.size());
    }

    static void pruneEmptyMilestones(DecomposedScopePlan* plan) {
        std::vector<DecomposedMilestone> kept;
        for (const auto& milestone : plan->milestones) {
            if (!milestone.workstreams.empty()) kept.push_back(milestone);
        }
        plan->milestones = kept;
    }

    static void dedupeIds(std::vector<std::string>* ids) {
        std::sort(ids->begin(), ids->end());
        ids->erase(std::unique(ids->begin(), ids->end()), ids->end());
    }
};
