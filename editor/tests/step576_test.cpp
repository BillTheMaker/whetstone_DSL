// Step 576: Scope and Milestone Decomposer (12 tests)

#include "ScopeMilestoneDecomposer.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static NormalizedRequirement req(const std::string& id,
                                 NormalizedRequirementKind kind,
                                 bool ambiguous = false) {
    NormalizedRequirement r;
    r.requirementId = id;
    r.kind = kind;
    r.normalizedText = "text " + id;
    r.anchor = "anchor";
    r.sourceLine = 1;
    r.ambiguous = ambiguous;
    return r;
}

void test_decompose_success() {
    TEST(decompose_success);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("goal-1", NormalizedRequirementKind::Goal));
    normalized.requirements.push_back(req("constraint-1", NormalizedRequirementKind::Constraint));
    normalized.requirements.push_back(req("dependency-1", NormalizedRequirementKind::Dependency));
    normalized.requirements.push_back(req("acceptance-1", NormalizedRequirementKind::Acceptance));

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(!plan.milestones.empty(), "milestones expected");
    PASS();
}

void test_decompose_fails_when_requirements_empty() {
    TEST(decompose_fails_when_requirements_empty);
    RequirementNormalizationResult normalized;
    DecomposedScopePlan plan;
    std::string error;
    CHECK(!ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should fail");
    CHECK(error == "requirements_empty", "wrong error");
    PASS();
}

void test_generates_intake_foundation_milestone() {
    TEST(generates_intake_foundation_milestone);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("goal-1", NormalizedRequirementKind::Goal));

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones[0].title == "Intake Foundation", "first milestone title mismatch");
    PASS();
}

void test_generates_execution_readiness_milestone_for_acceptance_dependency() {
    TEST(generates_execution_readiness_milestone_for_acceptance_dependency);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("dependency-1", NormalizedRequirementKind::Dependency));

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones[0].title == "Execution Readiness", "milestone title mismatch");
    PASS();
}

void test_workstream_includes_requirement_ids() {
    TEST(workstream_includes_requirement_ids);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("goal-1", NormalizedRequirementKind::Goal));
    normalized.requirements.push_back(req("constraint-1", NormalizedRequirementKind::Constraint));

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones[0].workstreams[0].requirementIds.size() == 2, "requirement id count mismatch");
    PASS();
}

void test_conflicts_create_architect_review_workstream() {
    TEST(conflicts_create_architect_review_workstream);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("constraint-1", NormalizedRequirementKind::Constraint));
    normalized.requirements.push_back(req("constraint-2", NormalizedRequirementKind::Constraint));
    normalized.conflicts.push_back({"constraint-1", "constraint-2", "constraint_contradiction", "detail"});

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones[0].workstreams.size() == 2, "review workstream expected");
    CHECK(plan.milestones[0].workstreams[1].title == "Architect Review", "review title mismatch");
    PASS();
}

void test_architect_review_workstream_dedupes_requirement_ids() {
    TEST(architect_review_workstream_dedupes_requirement_ids);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("constraint-1", NormalizedRequirementKind::Constraint));
    normalized.requirements.push_back(req("constraint-2", NormalizedRequirementKind::Constraint));
    normalized.conflicts.push_back({"constraint-1", "constraint-2", "constraint_contradiction", "detail"});
    normalized.conflicts.push_back({"constraint-1", "constraint-2", "constraint_contradiction", "detail-2"});

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones[0].workstreams[1].requirementIds.size() == 2, "ids should be deduped");
    PASS();
}

void test_ambiguous_requirements_raise_uncertainty() {
    TEST(ambiguous_requirements_raise_uncertainty);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("goal-1", NormalizedRequirementKind::Goal, false));
    normalized.requirements.push_back(req("goal-2", NormalizedRequirementKind::Goal, true));

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones[0].uncertaintyScore >= 25, "uncertainty should be elevated");
    PASS();
}

void test_acceptance_requirements_reduce_uncertainty() {
    TEST(acceptance_requirements_reduce_uncertainty);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("acceptance-1", NormalizedRequirementKind::Acceptance, false));

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones[0].uncertaintyScore <= 20, "acceptance should lower uncertainty");
    PASS();
}

void test_overall_uncertainty_is_averaged() {
    TEST(overall_uncertainty_is_averaged);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("goal-1", NormalizedRequirementKind::Goal, true));
    normalized.requirements.push_back(req("dependency-1", NormalizedRequirementKind::Dependency, false));

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.overallUncertainty > 0, "overall uncertainty should be set");
    PASS();
}

void test_only_relevant_milestones_are_kept() {
    TEST(only_relevant_milestones_are_kept);
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("goal-1", NormalizedRequirementKind::Goal));

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones.size() == 1, "only one milestone should remain");
    PASS();
}

void test_uncertainty_scores_stay_within_bounds() {
    TEST(uncertainty_scores_stay_within_bounds);
    RequirementNormalizationResult normalized;
    for (int i = 0; i < 20; ++i) {
        normalized.requirements.push_back(req("goal-" + std::to_string(i), NormalizedRequirementKind::Goal, true));
    }

    DecomposedScopePlan plan;
    std::string error;
    CHECK(ScopeMilestoneDecomposer::decompose(normalized, &plan, &error), "decompose should succeed");
    CHECK(plan.milestones[0].uncertaintyScore <= 100, "uncertainty upper bound exceeded");
    CHECK(plan.overallUncertainty <= 100, "overall uncertainty upper bound exceeded");
    PASS();
}

int main() {
    std::cout << "Step 576: Scope and Milestone Decomposer\n";

    test_decompose_success();                                      // 1
    test_decompose_fails_when_requirements_empty();                // 2
    test_generates_intake_foundation_milestone();                 // 3
    test_generates_execution_readiness_milestone_for_acceptance_dependency(); // 4
    test_workstream_includes_requirement_ids();                   // 5
    test_conflicts_create_architect_review_workstream();          // 6
    test_architect_review_workstream_dedupes_requirement_ids();   // 7
    test_ambiguous_requirements_raise_uncertainty();              // 8
    test_acceptance_requirements_reduce_uncertainty();            // 9
    test_overall_uncertainty_is_averaged();                       // 10
    test_only_relevant_milestones_are_kept();                     // 11
    test_uncertainty_scores_stay_within_bounds();                 // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
