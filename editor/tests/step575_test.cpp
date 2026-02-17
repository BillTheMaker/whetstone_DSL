// Step 575: Requirement Normalization and Conflict Detection (12 tests)

#include "RequirementNormalizationConflictDetector.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ParsedRequirementItem item(const std::string& text,
                                  const std::string& section,
                                  const std::string& anchor,
                                  int line) {
    ParsedRequirementItem i;
    i.text = text;
    i.sectionTitle = section;
    i.anchor = anchor;
    i.line = line;
    return i;
}

void test_normalize_from_all_categories() {
    TEST(normalize_from_all_categories);
    ParsedMarkdownSpec spec;
    spec.goals.push_back(item("Improve quality", "Goals", "goals", 3));
    spec.constraints.push_back(item("Must be deterministic", "Constraints", "constraints", 6));
    spec.dependencies.push_back(item("Need CMake", "Dependencies", "dependencies", 9));
    spec.acceptanceCriteria.push_back(item("All tests pass", "Acceptance", "acceptance", 12));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.requirements.size() == 4, "requirement count mismatch");
    PASS();
}

void test_normalized_text_is_lowercased_and_compacted() {
    TEST(normalized_text_is_lowercased_and_compacted);
    ParsedMarkdownSpec spec;
    spec.goals.push_back(item("  Keep, HEADER-Only!   ", "Goals", "goals", 4));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.requirements[0].normalizedText == "keep header only", "normalized text mismatch");
    PASS();
}

void test_requirement_ids_are_stable_per_category() {
    TEST(requirement_ids_are_stable_per_category);
    ParsedMarkdownSpec spec;
    spec.goals.push_back(item("Goal A", "Goals", "goals", 1));
    spec.goals.push_back(item("Goal B", "Goals", "goals", 2));
    spec.constraints.push_back(item("Constraint A", "Constraints", "constraints", 3));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.requirements[0].requirementId == "goal-1", "goal-1 expected");
    CHECK(result.requirements[1].requirementId == "goal-2", "goal-2 expected");
    CHECK(result.requirements[2].requirementId == "constraint-1", "constraint-1 expected");
    PASS();
}

void test_preserves_anchor_and_source_line() {
    TEST(preserves_anchor_and_source_line);
    ParsedMarkdownSpec spec;
    spec.constraints.push_back(item("Must stay small", "Constraints", "constraints", 44));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.requirements[0].anchor == "constraints", "anchor mismatch");
    CHECK(result.requirements[0].sourceLine == 44, "source line mismatch");
    PASS();
}

void test_ambiguous_language_is_flagged() {
    TEST(ambiguous_language_is_flagged);
    ParsedMarkdownSpec spec;
    spec.goals.push_back(item("Maybe improve performance somehow", "Goals", "goals", 5));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.requirements[0].ambiguous, "ambiguous flag should be true");
    PASS();
}

void test_clear_language_is_not_flagged_ambiguous() {
    TEST(clear_language_is_not_flagged_ambiguous);
    ParsedMarkdownSpec spec;
    spec.goals.push_back(item("Improve parser throughput by 20 percent", "Goals", "goals", 5));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(!result.requirements[0].ambiguous, "ambiguous flag should be false");
    PASS();
}

void test_detects_conflict_between_positive_and_negative_constraints() {
    TEST(detects_conflict_between_positive_and_negative_constraints);
    ParsedMarkdownSpec spec;
    spec.constraints.push_back(item("Enable dynamic plugins", "Constraints", "constraints", 6));
    spec.constraints.push_back(item("Do not enable dynamic plugins", "Constraints", "constraints", 7));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.conflicts.size() == 1, "one conflict expected");
    CHECK(result.conflicts[0].conflictType == "constraint_contradiction", "conflict type mismatch");
    PASS();
}

void test_no_conflict_for_unrelated_constraints() {
    TEST(no_conflict_for_unrelated_constraints);
    ParsedMarkdownSpec spec;
    spec.constraints.push_back(item("Enable syntax highlighting", "Constraints", "constraints", 6));
    spec.constraints.push_back(item("Require offline mode", "Constraints", "constraints", 7));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.conflicts.empty(), "no conflicts expected");
    PASS();
}

void test_no_conflict_when_both_constraints_negative() {
    TEST(no_conflict_when_both_constraints_negative);
    ParsedMarkdownSpec spec;
    spec.constraints.push_back(item("Do not allow network", "Constraints", "constraints", 6));
    spec.constraints.push_back(item("Never allow telemetry", "Constraints", "constraints", 7));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.conflicts.empty(), "both negative constraints should not conflict");
    PASS();
}

void test_conflict_records_requirement_ids() {
    TEST(conflict_records_requirement_ids);
    ParsedMarkdownSpec spec;
    spec.constraints.push_back(item("Use cache", "Constraints", "constraints", 6));
    spec.constraints.push_back(item("Do not use cache", "Constraints", "constraints", 7));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.conflicts[0].leftRequirementId == "constraint-1", "left id mismatch");
    CHECK(result.conflicts[0].rightRequirementId == "constraint-2", "right id mismatch");
    PASS();
}

void test_no_requirements_fails() {
    TEST(no_requirements_fails);
    ParsedMarkdownSpec spec;
    RequirementNormalizationResult result;
    std::string error;
    CHECK(!RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should fail");
    CHECK(error == "no_requirements_found", "wrong error");
    PASS();
}

void test_non_constraint_items_do_not_generate_conflicts() {
    TEST(non_constraint_items_do_not_generate_conflicts);
    ParsedMarkdownSpec spec;
    spec.goals.push_back(item("Enable cache", "Goals", "goals", 3));
    spec.goals.push_back(item("Do not enable cache", "Goals", "goals", 4));

    RequirementNormalizationResult result;
    std::string error;
    CHECK(RequirementNormalizationConflictDetector::normalize(spec, &result, &error), "normalize should succeed");
    CHECK(result.conflicts.empty(), "goal contradiction should not be marked as constraint conflict");
    PASS();
}

int main() {
    std::cout << "Step 575: Requirement Normalization and Conflict Detection\n";

    test_normalize_from_all_categories();                              // 1
    test_normalized_text_is_lowercased_and_compacted();               // 2
    test_requirement_ids_are_stable_per_category();                   // 3
    test_preserves_anchor_and_source_line();                          // 4
    test_ambiguous_language_is_flagged();                             // 5
    test_clear_language_is_not_flagged_ambiguous();                   // 6
    test_detects_conflict_between_positive_and_negative_constraints();// 7
    test_no_conflict_for_unrelated_constraints();                     // 8
    test_no_conflict_when_both_constraints_negative();                // 9
    test_conflict_records_requirement_ids();                          // 10
    test_no_requirements_fails();                                     // 11
    test_non_constraint_items_do_not_generate_conflicts();            // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
