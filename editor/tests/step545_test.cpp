// Step 545: Context Bundle Minimizer (12 tests)

#include "ContextBundleMinimizer.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool contains(const std::vector<std::string>& v, const std::string& needle) {
    for (const auto& x : v) if (x == needle) return true;
    return false;
}

static ContextBundleInput sampleInput(bool narrow = true) {
    ContextBundleInput i;
    i.taskitemId = "ti-545";
    i.contractTargets = {"Function"};
    i.contractSymbols = {"cursor", "EditorState"};
    i.contractOps = {"rename"};
    i.fileContext = {
        "file:main.cpp",
        "ast:Function cursor update block",
        "diag:none",
        "note:unrelated_ui_theme"
    };
    i.projectContext = {
        "module:EditorState contract cursor",
        "module:Theme purple gradient",
        "module:Function signatures"
    };
    i.dependencyContext = {
        "dep:tree_sitter Function",
        "dep:sdl2 window",
        "dep:cursor_utils"
    };
    i.narrowOperation = narrow;
    return i;
}

void test_narrow_mode_keeps_relevant_file_context() {
    TEST(narrow_mode_keeps_relevant_file_context);
    auto out = ContextBundleMinimizer::minimize(sampleInput(true));
    CHECK(contains(out.minimalContext, "ast:Function cursor update block"), "relevant file context should remain");
    PASS();
}

void test_narrow_mode_drops_unrelated_project_context() {
    TEST(narrow_mode_drops_unrelated_project_context);
    auto out = ContextBundleMinimizer::minimize(sampleInput(true));
    CHECK(contains(out.removedContext, "module:Theme purple gradient"), "unrelated project context should drop");
    PASS();
}

void test_narrow_mode_drops_unrelated_dependency_context() {
    TEST(narrow_mode_drops_unrelated_dependency_context);
    auto out = ContextBundleMinimizer::minimize(sampleInput(true));
    CHECK(contains(out.removedContext, "dep:sdl2 window"), "unrelated dependency context should drop");
    PASS();
}

void test_nonnarrow_mode_keeps_structural_project_context() {
    TEST(nonnarrow_mode_keeps_structural_project_context);
    auto in = sampleInput(false);
    auto out = ContextBundleMinimizer::minimize(in);
    CHECK(contains(out.minimalContext, "module:Theme purple gradient") == false,
          "non-structural unrelated module should still drop");
    CHECK(contains(out.minimalContext, "module:Function signatures"),
          "relevant project module should remain");
    PASS();
}

void test_nonnarrow_mode_keeps_structural_file_entries() {
    TEST(nonnarrow_mode_keeps_structural_file_entries);
    auto in = sampleInput(false);
    auto out = ContextBundleMinimizer::minimize(in);
    CHECK(contains(out.minimalContext, "file:main.cpp"), "file structural entry should remain");
    CHECK(contains(out.minimalContext, "diag:none"), "diag structural entry should remain");
    PASS();
}

void test_contract_symbols_drive_relevance() {
    TEST(contract_symbols_drive_relevance);
    auto in = sampleInput(true);
    in.projectContext.push_back("module:EditorState snapshots");
    auto out = ContextBundleMinimizer::minimize(in);
    CHECK(contains(out.minimalContext, "module:EditorState snapshots"), "symbol-linked module should remain");
    PASS();
}

void test_contract_ops_drive_relevance() {
    TEST(contract_ops_drive_relevance);
    auto in = sampleInput(true);
    in.projectContext.push_back("module:rename strategy docs");
    auto out = ContextBundleMinimizer::minimize(in);
    CHECK(contains(out.minimalContext, "module:rename strategy docs"), "op-linked module should remain");
    PASS();
}

void test_dedupe_applies_to_minimal_context() {
    TEST(dedupe_applies_to_minimal_context);
    auto in = sampleInput(true);
    in.fileContext.push_back("ast:Function cursor update block");
    auto out = ContextBundleMinimizer::minimize(in);
    int count = 0;
    for (const auto& x : out.minimalContext) if (x == "ast:Function cursor update block") ++count;
    CHECK(count == 1, "minimal context should be deduped");
    PASS();
}

void test_dedupe_applies_to_removed_context() {
    TEST(dedupe_applies_to_removed_context);
    auto in = sampleInput(true);
    in.projectContext.push_back("module:Theme purple gradient");
    auto out = ContextBundleMinimizer::minimize(in);
    int count = 0;
    for (const auto& x : out.removedContext) if (x == "module:Theme purple gradient") ++count;
    CHECK(count == 1, "removed context should be deduped");
    PASS();
}

void test_empty_context_inputs_return_empty_outputs() {
    TEST(empty_context_inputs_return_empty_outputs);
    ContextBundleInput in;
    auto out = ContextBundleMinimizer::minimize(in);
    CHECK(out.minimalContext.empty(), "minimal should be empty");
    CHECK(out.removedContext.empty(), "removed should be empty");
    PASS();
}

void test_irrelevant_file_note_dropped_even_if_nonnarrow() {
    TEST(irrelevant_file_note_dropped_even_if_nonnarrow);
    auto in = sampleInput(false);
    auto out = ContextBundleMinimizer::minimize(in);
    CHECK(contains(out.removedContext, "note:unrelated_ui_theme"), "irrelevant file note should drop");
    PASS();
}

void test_dependency_relevance_by_symbol_match() {
    TEST(dependency_relevance_by_symbol_match);
    auto in = sampleInput(true);
    in.dependencyContext.push_back("dep:EditorState_helpers");
    auto out = ContextBundleMinimizer::minimize(in);
    CHECK(contains(out.minimalContext, "dep:EditorState_helpers"), "symbol-matched dependency should remain");
    PASS();
}

int main() {
    std::cout << "Step 545: Context Bundle Minimizer\n";

    test_narrow_mode_keeps_relevant_file_context();           // 1
    test_narrow_mode_drops_unrelated_project_context();       // 2
    test_narrow_mode_drops_unrelated_dependency_context();    // 3
    test_nonnarrow_mode_keeps_structural_project_context();   // 4
    test_nonnarrow_mode_keeps_structural_file_entries();      // 5
    test_contract_symbols_drive_relevance();                  // 6
    test_contract_ops_drive_relevance();                      // 7
    test_dedupe_applies_to_minimal_context();                 // 8
    test_dedupe_applies_to_removed_context();                 // 9
    test_empty_context_inputs_return_empty_outputs();         // 10
    test_irrelevant_file_note_dropped_even_if_nonnarrow();   // 11
    test_dependency_relevance_by_symbol_match();              // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
