// Step 578: Phase 32a Integration (8 tests)

#include "Phase32aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Phase32aResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

static std::string validMarkdown() {
    return R"(## Goals
- Improve intake reliability

## Constraints
- Enable deterministic parsing
- Do not enable deterministic parsing

## Dependencies
- CMake

## Acceptance Criteria
- Parse all required sections
)";
}

void test_full_cycle_passes_with_auto_approval() {
    TEST(full_cycle_passes_with_auto_approval);
    Phase32aInput input;
    input.markdown = validMarkdown();
    auto result = Phase32aIntegration::run(input);
    CHECK(result.phase32aPass, "phase should pass");
    CHECK(result.parseReady, "parse should be ready");
    CHECK(result.normalizationReady, "normalization should be ready");
    CHECK(result.decompositionReady, "decomposition should be ready");
    CHECK(result.reviewReady, "review should be ready");
    CHECK(hasNote(result, "phase32a:markdown_intake_cycle_ready"), "success note missing");
    PASS();
}

void test_parse_failure_blocks_phase() {
    TEST(parse_failure_blocks_phase);
    Phase32aInput input;
    input.markdown = "";
    auto result = Phase32aIntegration::run(input);
    CHECK(!result.phase32aPass, "phase should fail");
    CHECK(hasNote(result, "fail:markdown_parse"), "parse failure note expected");
    PASS();
}

void test_normalization_failure_blocks_phase() {
    TEST(normalization_failure_blocks_phase);
    Phase32aInput input;
    input.markdown = "## Notes\ntext only";
    auto result = Phase32aIntegration::run(input);
    CHECK(!result.phase32aPass, "phase should fail");
    CHECK(hasNote(result, "fail:normalization"), "normalization failure note expected");
    PASS();
}

void test_review_load_failure_when_no_constraints() {
    TEST(review_load_failure_when_no_constraints);
    Phase32aInput input;
    input.markdown = R"(## Goals
- goal
## Acceptance Criteria
- done
)";
    auto result = Phase32aIntegration::run(input);
    CHECK(!result.phase32aPass, "phase should fail");
    CHECK(hasNote(result, "fail:review_load"), "review load failure note expected");
    PASS();
}

void test_modify_mode_passes_and_counts_modified() {
    TEST(modify_mode_passes_and_counts_modified);
    Phase32aInput input;
    input.markdown = validMarkdown();
    input.autoApproveConstraints = false;
    auto result = Phase32aIntegration::run(input);
    CHECK(result.phase32aPass, "phase should pass");
    CHECK(result.reviewSummary.modified == 2, "modified count mismatch");
    PASS();
}

void test_force_reject_first_constraint_counts_rejected() {
    TEST(force_reject_first_constraint_counts_rejected);
    Phase32aInput input;
    input.markdown = validMarkdown();
    input.forceRejectFirstConstraint = true;
    auto result = Phase32aIntegration::run(input);
    CHECK(result.phase32aPass, "phase should still pass");
    CHECK(result.reviewSummary.rejected == 1, "one rejected expected");
    CHECK(result.reviewSummary.approved == 1, "one approved expected");
    PASS();
}

void test_conflict_count_exposed() {
    TEST(conflict_count_exposed);
    Phase32aInput input;
    input.markdown = validMarkdown();
    auto result = Phase32aIntegration::run(input);
    CHECK(result.phase32aPass, "phase should pass");
    CHECK(result.conflictCount >= 1, "conflict count should be exposed");
    PASS();
}

void test_parsed_section_count_exposed() {
    TEST(parsed_section_count_exposed);
    Phase32aInput input;
    input.markdown = validMarkdown();
    auto result = Phase32aIntegration::run(input);
    CHECK(result.phase32aPass, "phase should pass");
    CHECK(result.parsedSectionCount == 4, "section count mismatch");
    PASS();
}

int main() {
    std::cout << "Step 578: Phase 32a Integration\n";

    test_full_cycle_passes_with_auto_approval();     // 1
    test_parse_failure_blocks_phase();               // 2
    test_normalization_failure_blocks_phase();       // 3
    test_review_load_failure_when_no_constraints();  // 4
    test_modify_mode_passes_and_counts_modified();   // 5
    test_force_reject_first_constraint_counts_rejected(); // 6
    test_conflict_count_exposed();                   // 7
    test_parsed_section_count_exposed();             // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
