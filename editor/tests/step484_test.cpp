// Step 484: Prior-Step Context Injector Tests (unit, integration)

#include "PriorStepContextInjector.h"

#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_extracts_type_names_from_prior_headers() {
    TEST(extracts_type_names_from_prior_headers);
    auto ctx = PriorStepContextInjector::build({
        "editor/src/StepSpecExpander.h",
        "editor/src/ProjectConventionAnalyzer.h"
    });
    bool foundStepSpec = false, foundConventions = false;
    for (const auto& t : ctx.priorApis) {
        if (t.name == "StepSpec") foundStepSpec = true;
        if (t.name == "ProjectConventions") foundConventions = true;
    }
    CHECK(foundStepSpec, "missing StepSpec");
    CHECK(foundConventions, "missing ProjectConventions");
    PASS();
}

void test_extracts_method_signatures_from_headers() {
    TEST(extracts_method_signatures_from_headers);
    auto ctx = PriorStepContextInjector::build({"editor/src/StepSpecExpander.h"});
    CHECK(!ctx.priorApis.empty(), "expected extracted APIs");
    bool hasExpand = false;
    for (const auto& m : ctx.priorApis[0].methods) {
        if (m.signature.find("expand(") != std::string::npos) hasExpand = true;
    }
    CHECK(hasExpand, "missing expand method signature");
    PASS();
}

void test_builds_include_dependency_graph() {
    TEST(builds_include_dependency_graph);
    auto path = std::string("editor/src/StepSpecExpander.h");
    auto ctx = PriorStepContextInjector::build({path});
    CHECK(ctx.includeDependencies.count(path) == 1, "missing include graph entry");
    PASS();
}

void test_detects_naming_patterns_from_prior_types() {
    TEST(detects_naming_patterns_from_prior_types);
    auto ctx = PriorStepContextInjector::build({
        "editor/src/StepSpecExpander.h",
        "editor/src/WorkflowState.h"
    });
    bool hasSpec = false, hasState = false;
    for (const auto& p : ctx.namingPatterns) {
        if (p == "Suffix:Spec") hasSpec = true;
        if (p == "Suffix:State") hasState = true;
    }
    CHECK(hasSpec, "expected Spec suffix pattern");
    CHECK(hasState, "expected State suffix pattern");
    PASS();
}

void test_condensed_brief_contains_type_and_pattern_summary() {
    TEST(condensed_brief_contains_type_and_pattern_summary);
    auto ctx = PriorStepContextInjector::build({"editor/src/StepSpecExpander.h"});
    auto brief = ctx.condensedBrief();
    CHECK(brief.find("types:") != std::string::npos, "missing types section");
    CHECK(brief.find("patterns:") != std::string::npos, "missing patterns section");
    PASS();
}

void test_method_signatures_are_declaration_level_not_implementation_blocks() {
    TEST(method_signatures_are_declaration_level_not_implementation_blocks);
    auto ctx = PriorStepContextInjector::build({"editor/src/StepSpecExpander.h"});
    CHECK(!ctx.priorApis.empty(), "missing api summaries");
    for (const auto& m : ctx.priorApis[0].methods) {
        CHECK(m.signature.find("{") == std::string::npos, "unexpected implementation brace");
    }
    PASS();
}

void test_missing_header_path_reports_warning() {
    TEST(missing_header_path_reports_warning);
    auto ctx = PriorStepContextInjector::build({"editor/src/DOES_NOT_EXIST_484.h"});
    CHECK(!ctx.warnings.empty(), "expected missing file warning");
    PASS();
}

void test_integration_multiple_headers_yield_nonempty_context_brief() {
    TEST(integration_multiple_headers_yield_nonempty_context_brief);
    auto ctx = PriorStepContextInjector::build({
        "editor/src/StepSpecExpander.h",
        "editor/src/ProjectConventionAnalyzer.h",
        "editor/src/PriorStepContextInjector.h"
    });
    CHECK(ctx.priorApis.size() >= 3, "expected at least three API summaries");
    CHECK(!ctx.condensedBrief().empty(), "expected non-empty brief");
    PASS();
}

int main() {
    std::cout << "Step 484: Prior-Step Context Injector Tests\n";

    test_extracts_type_names_from_prior_headers();                         // 1
    test_extracts_method_signatures_from_headers();                        // 2
    test_builds_include_dependency_graph();                                // 3
    test_detects_naming_patterns_from_prior_types();                       // 4
    test_condensed_brief_contains_type_and_pattern_summary();              // 5
    test_method_signatures_are_declaration_level_not_implementation_blocks(); // 6
    test_missing_header_path_reports_warning();                            // 7
    test_integration_multiple_headers_yield_nonempty_context_brief();      // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
