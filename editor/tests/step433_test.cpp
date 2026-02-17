// Step 433: Inline Annotation Badges Tests (12 tests)

#include "CodeAnnotationBadges.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::vector<AnnotationBadgeInput> sampleInputs() {
    return {
        {10, "IntentAnnotation", "Parse auth tokens", "n1"},
        {10, "RiskAnnotation", "High external exposure", "n1"},
        {20, "ComplexityAnnotation", "Cyclomatic=12", "n2"},
        {30, "ReviewAnnotation", "Human review required", "n3"}
    };
}

void test_build_badges_count_matches_input() {
    TEST(build_badges_count_matches_input);
    auto badges = CodeAnnotationBadges::buildBadges(sampleInputs());
    CHECK(badges.size() == sampleInputs().size(), "badge count mismatch");
    PASS();
}

void test_badges_sorted_by_line_then_type() {
    TEST(badges_sorted_by_line_then_type);
    auto badges = CodeAnnotationBadges::buildBadges({
        {30, "RiskAnnotation", "r", "n3"},
        {10, "IntentAnnotation", "i", "n1"},
        {10, "ComplexityAnnotation", "c", "n1"}
    });
    CHECK(badges[0].line == 10 && badges[1].line == 10 && badges[2].line == 30,
          "line sort mismatch");
    PASS();
}

void test_icon_mapping_for_known_types() {
    TEST(icon_mapping_for_known_types);
    CHECK(CodeAnnotationBadges::iconForType("IntentAnnotation") == "I", "intent icon");
    CHECK(CodeAnnotationBadges::iconForType("ComplexityAnnotation") == "C", "complexity icon");
    CHECK(CodeAnnotationBadges::iconForType("RiskAnnotation") == "R", "risk icon");
    PASS();
}

void test_color_mapping_for_known_types() {
    TEST(color_mapping_for_known_types);
    CHECK(CodeAnnotationBadges::colorForType("IntentAnnotation") == "#1ABC9C", "intent color");
    CHECK(CodeAnnotationBadges::colorForType("RiskAnnotation") == "#E74C3C", "risk color");
    PASS();
}

void test_unknown_type_fallback_icon_and_color() {
    TEST(unknown_type_fallback_icon_and_color);
    CHECK(CodeAnnotationBadges::iconForType("UnknownAnnotation") == "A", "fallback icon");
    CHECK(CodeAnnotationBadges::colorForType("UnknownAnnotation") == "#95A5A6", "fallback color");
    PASS();
}

void test_stack_by_line_groups_multiple_badges_same_line() {
    TEST(stack_by_line_groups_multiple_badges_same_line);
    auto badges = CodeAnnotationBadges::buildBadges(sampleInputs());
    auto stacks = CodeAnnotationBadges::stackByLine(badges);
    int line10Count = 0;
    for (const auto& s : stacks) if (s.line == 10) line10Count = (int)s.badges.size();
    CHECK(line10Count == 2, "line 10 should have two stacked badges");
    PASS();
}

void test_stack_by_line_sorted_by_line() {
    TEST(stack_by_line_sorted_by_line);
    auto badges = CodeAnnotationBadges::buildBadges(sampleInputs());
    auto stacks = CodeAnnotationBadges::stackByLine(badges);
    CHECK(stacks.size() >= 3, "expected three lines");
    CHECK(stacks[0].line < stacks[1].line, "stacks should be sorted by line");
    PASS();
}

void test_expand_detail_contains_type_node_and_detail() {
    TEST(expand_detail_contains_type_node_and_detail);
    auto badges = CodeAnnotationBadges::buildBadges(sampleInputs());
    auto text = CodeAnnotationBadges::expandDetail(badges[0]);
    CHECK(text.find(badges[0].type) != std::string::npos, "missing type");
    CHECK(text.find(badges[0].nodeId) != std::string::npos, "missing node");
    CHECK(text.find(badges[0].detail) != std::string::npos, "missing detail");
    PASS();
}

void test_badge_preserves_line_and_node_id() {
    TEST(badge_preserves_line_and_node_id);
    auto badges = CodeAnnotationBadges::buildBadges(sampleInputs());
    CHECK(badges[0].line > 0, "line missing");
    CHECK(!badges[0].nodeId.empty(), "node id missing");
    PASS();
}

void test_empty_input_returns_empty_badges() {
    TEST(empty_input_returns_empty_badges);
    auto badges = CodeAnnotationBadges::buildBadges({});
    CHECK(badges.empty(), "expected empty badges");
    PASS();
}

void test_empty_badges_return_empty_stacks() {
    TEST(empty_badges_return_empty_stacks);
    auto stacks = CodeAnnotationBadges::stackByLine({});
    CHECK(stacks.empty(), "expected empty stacks");
    PASS();
}

void test_review_annotation_has_review_icon() {
    TEST(review_annotation_has_review_icon);
    auto badges = CodeAnnotationBadges::buildBadges({{5, "ReviewAnnotation", "review me", "n1"}});
    CHECK(!badges.empty(), "badge missing");
    CHECK(badges[0].icon == "V", "review icon mismatch");
    PASS();
}

int main() {
    std::cout << "Step 433: Inline Annotation Badges Tests\n";

    test_build_badges_count_matches_input();            // 1
    test_badges_sorted_by_line_then_type();             // 2
    test_icon_mapping_for_known_types();                // 3
    test_color_mapping_for_known_types();               // 4
    test_unknown_type_fallback_icon_and_color();        // 5
    test_stack_by_line_groups_multiple_badges_same_line();// 6
    test_stack_by_line_sorted_by_line();                // 7
    test_expand_detail_contains_type_node_and_detail(); // 8
    test_badge_preserves_line_and_node_id();            // 9
    test_empty_input_returns_empty_badges();            // 10
    test_empty_badges_return_empty_stacks();            // 11
    test_review_annotation_has_review_icon();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
