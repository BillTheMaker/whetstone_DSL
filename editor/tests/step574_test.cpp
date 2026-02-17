// Step 574: Markdown Spec Parser (12 tests)

#include "MarkdownSpecParser.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static const char* kSample = R"(# Product Spec

## Goals
- Improve parser reliability
- Preserve traceability

## Constraints
- Must remain header-only
- Keep tests deterministic

## Dependencies
- CMake
- Tree-sitter runtime

## Acceptance Criteria
- Extract all goal bullets
- Include section anchor and source line
)";

void test_parse_sample_success() {
    TEST(parse_sample_success);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(kSample, &spec, &error), "parse should succeed");
    CHECK(spec.sections.size() == 4, "section count mismatch");
    PASS();
}

void test_parse_extracts_goals() {
    TEST(parse_extracts_goals);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(kSample, &spec, &error), "parse should succeed");
    CHECK(spec.goals.size() == 2, "goal count mismatch");
    CHECK(spec.goals[0].text == "Improve parser reliability", "first goal mismatch");
    PASS();
}

void test_parse_extracts_constraints() {
    TEST(parse_extracts_constraints);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(kSample, &spec, &error), "parse should succeed");
    CHECK(spec.constraints.size() == 2, "constraint count mismatch");
    CHECK(spec.constraints[1].text == "Keep tests deterministic", "second constraint mismatch");
    PASS();
}

void test_parse_extracts_dependencies() {
    TEST(parse_extracts_dependencies);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(kSample, &spec, &error), "parse should succeed");
    CHECK(spec.dependencies.size() == 2, "dependency count mismatch");
    CHECK(spec.dependencies[0].text == "CMake", "first dependency mismatch");
    PASS();
}

void test_parse_extracts_acceptance_criteria() {
    TEST(parse_extracts_acceptance_criteria);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(kSample, &spec, &error), "parse should succeed");
    CHECK(spec.acceptanceCriteria.size() == 2, "acceptance count mismatch");
    CHECK(spec.acceptanceCriteria[1].text == "Include section anchor and source line", "acceptance text mismatch");
    PASS();
}

void test_anchor_generation_from_heading() {
    TEST(anchor_generation_from_heading);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(kSample, &spec, &error), "parse should succeed");
    CHECK(spec.sections[0].anchor == "goals", "goals anchor mismatch");
    CHECK(spec.sections[3].anchor == "acceptance-criteria", "acceptance anchor mismatch");
    PASS();
}

void test_requirement_items_include_traceability_fields() {
    TEST(requirement_items_include_traceability_fields);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(kSample, &spec, &error), "parse should succeed");
    CHECK(spec.goals[0].sectionTitle == "Goals", "section title mismatch");
    CHECK(spec.goals[0].anchor == "goals", "anchor mismatch");
    CHECK(spec.goals[0].line > 0, "line should be positive");
    PASS();
}

void test_ignores_bullets_outside_sections() {
    TEST(ignores_bullets_outside_sections);
    const char* markdown = R"(- orphan bullet
## Goals
- valid goal
)";
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(markdown, &spec, &error), "parse should succeed");
    CHECK(spec.goals.size() == 1, "orphan bullet should be ignored");
    PASS();
}

void test_empty_markdown_fails() {
    TEST(empty_markdown_fails);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(!MarkdownSpecParser::parse("", &spec, &error), "empty markdown should fail");
    CHECK(error == "markdown_empty", "wrong error");
    PASS();
}

void test_markdown_without_sections_fails() {
    TEST(markdown_without_sections_fails);
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(!MarkdownSpecParser::parse("# Title\n- bullet", &spec, &error), "markdown without sections should fail");
    CHECK(error == "no_sections_found", "wrong error");
    PASS();
}

void test_whitespace_in_bullets_is_trimmed() {
    TEST(whitespace_in_bullets_is_trimmed);
    const char* markdown = R"(## Goals
-    spaced goal text    
)";
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(markdown, &spec, &error), "parse should succeed");
    CHECK(spec.goals.size() == 1, "goal count mismatch");
    CHECK(spec.goals[0].text == "spaced goal text", "goal text should be trimmed");
    PASS();
}

void test_subsection_heading_without_double_hash_is_ignored_for_section_split() {
    TEST(subsection_heading_without_double_hash_is_ignored_for_section_split);
    const char* markdown = R"(## Goals
### Details
- goal one
)";
    ParsedMarkdownSpec spec;
    std::string error;
    CHECK(MarkdownSpecParser::parse(markdown, &spec, &error), "parse should succeed");
    CHECK(spec.sections.size() == 1, "only one top-level section expected");
    CHECK(spec.goals.size() == 1, "goal should still be parsed");
    PASS();
}

int main() {
    std::cout << "Step 574: Markdown Spec Parser\n";

    test_parse_sample_success();                                      // 1
    test_parse_extracts_goals();                                      // 2
    test_parse_extracts_constraints();                                // 3
    test_parse_extracts_dependencies();                               // 4
    test_parse_extracts_acceptance_criteria();                        // 5
    test_anchor_generation_from_heading();                            // 6
    test_requirement_items_include_traceability_fields();             // 7
    test_ignores_bullets_outside_sections();                          // 8
    test_empty_markdown_fails();                                      // 9
    test_markdown_without_sections_fails();                           // 10
    test_whitespace_in_bullets_is_trimmed();                          // 11
    test_subsection_heading_without_double_hash_is_ignored_for_section_split(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
