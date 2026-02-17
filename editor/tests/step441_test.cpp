// Step 441: Modernization Workflow Generation Tests (12 tests)

#include "ModernizationWorkflow.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ModernizationWorkflow buildWorkflow(const std::string& src,
                                           const std::string& lang,
                                           const std::string& file,
                                           const std::string& target = "") {
    auto legacy = LegacyIdiomDetector::analyzeFile(src, lang, file);
    auto safety = SafetyAuditor::audit(src, lang, file);
    auto report = ModernizationSuggester::suggest(legacy, safety, target);
    return ModernizationWorkflowGenerator::generate(src, report);
}

void test_workflow_items_created_from_suggestions() {
    TEST(workflow_items_created_from_suggestions);
    auto wf = buildWorkflow("void f(){ char b[32]; gets(b); sprintf(b,\"x\"); }", "c", "a.c");
    CHECK(wf.items.size() >= 2, "expected at least 2 work items");
    CHECK(wf.hasItemFor("gets"), "expected gets work item");
    CHECK(wf.hasItemFor("sprintf"), "expected sprintf work item");
    PASS();
}

void test_quick_wins_routed_deterministic() {
    TEST(quick_wins_routed_deterministic);
    auto wf = buildWorkflow("void f(){ char b[32]; gets(b); strcpy(b,\"x\"); }", "c", "b.c");
    int det = wf.countByRouting(WorkItemRouting::Deterministic);
    CHECK(det >= 2, "expected at least 2 deterministic items for quick wins");
    PASS();
}

void test_deep_refactors_routed_human() {
    TEST(deep_refactors_routed_human);
    auto wf = buildWorkflow("void f(){ start: goto start; }", "c", "c.c");
    int human = wf.countByRouting(WorkItemRouting::Human);
    CHECK(human >= 1, "expected at least 1 human-routed item for goto refactor");
    PASS();
}

void test_moderate_routed_llm() {
    TEST(moderate_routed_llm);
    auto wf = buildWorkflow("void f(){ int* p=(int*)malloc(4); free(p); }", "cpp", "d.cpp");
    int llm = wf.countByRouting(WorkItemRouting::LLM);
    CHECK(llm >= 1, "expected at least 1 LLM-routed item for moderate effort");
    PASS();
}

void test_ordering_safe_first_risky_last() {
    TEST(ordering_safe_first_risky_last);
    const std::string src =
        "void f(){\n"
        "  char b[32]; gets(b); strcpy(b,\"x\");\n"
        "  start: goto start;\n"
        "}\n";
    auto wf = buildWorkflow(src, "c", "e.c");
    auto ordered = wf.itemsInOrder();
    CHECK(ordered.size() >= 3, "expected at least 3 items");
    // First items should be priority 0 (quick wins), last should be priority 2 (deep)
    CHECK(ordered.front().priority <= ordered.back().priority,
          "safe changes should come before risky ones");
    CHECK(ordered.front().effort == ModernizeEffort::QuickWin,
          "first item should be a quick win");
    PASS();
}

void test_deep_refactors_depend_on_quick_wins() {
    TEST(deep_refactors_depend_on_quick_wins);
    const std::string src =
        "void f(){\n"
        "  char b[32]; gets(b);\n"
        "  start: goto start;\n"
        "}\n";
    auto wf = buildWorkflow(src, "c", "f.c");
    // Find the goto (deep refactor) item
    bool foundDep = false;
    for (const auto& item : wf.items) {
        if (item.effort == ModernizeEffort::DeepRefactor) {
            CHECK(!item.dependsOn.empty(),
                  "deep refactor should depend on quick wins");
            foundDep = true;
        }
    }
    CHECK(foundDep, "expected a deep refactor item");
    PASS();
}

void test_skeleton_generated() {
    TEST(skeleton_generated);
    auto wf = buildWorkflow("void f(){ char b[32]; gets(b); }", "c", "g.c");
    CHECK(!wf.skeleton.skeletonSource.empty(), "skeleton source should not be empty");
    CHECK(wf.skeleton.originalSource != wf.skeleton.skeletonSource,
          "skeleton should differ from original");
    PASS();
}

void test_skeleton_has_modernize_comments() {
    TEST(skeleton_has_modernize_comments);
    auto wf = buildWorkflow("void f(){ char b[32]; gets(b); }", "c", "h.c");
    CHECK(wf.skeleton.skeletonSource.find("@Modernize") != std::string::npos,
          "skeleton should contain @Modernize comments");
    PASS();
}

void test_skeleton_annotations_collected() {
    TEST(skeleton_annotations_collected);
    auto wf = buildWorkflow("void f(){ char b[32]; gets(b); sprintf(b,\"x\"); }", "c", "i.c");
    CHECK(wf.skeleton.annotations.size() >= 2, "expected at least 2 annotations");
    PASS();
}

void test_work_item_ids_unique() {
    TEST(work_item_ids_unique);
    const std::string src =
        "void f(){\n"
        "  gets(b); sprintf(b,\"x\"); strcpy(b,\"y\");\n"
        "  int* p=(int*)malloc(4); free(p);\n"
        "}\n";
    auto wf = buildWorkflow(src, "c", "j.c");
    for (size_t i = 0; i < wf.items.size(); ++i)
        for (size_t j = i + 1; j < wf.items.size(); ++j)
            CHECK(wf.items[i].id != wf.items[j].id, "work item IDs must be unique");
    PASS();
}

void test_workflow_file_metadata() {
    TEST(workflow_file_metadata);
    auto wf = buildWorkflow("void f(){ gets(b); }", "c", "meta.c", "rust");
    CHECK(wf.filePath == "meta.c", "expected filePath");
    CHECK(wf.sourceLanguage == "c", "expected source language");
    CHECK(wf.targetLanguage == "rust", "expected target language");
    PASS();
}

void test_empty_source_produces_empty_workflow() {
    TEST(empty_source_produces_empty_workflow);
    auto wf = buildWorkflow("int main(){ return 0; }", "c", "clean.c");
    CHECK(wf.items.empty(), "clean code should produce no work items");
    PASS();
}

int main() {
    std::cout << "Step 441: Modernization Workflow Generation Tests\n";

    test_workflow_items_created_from_suggestions();   // 1
    test_quick_wins_routed_deterministic();           // 2
    test_deep_refactors_routed_human();               // 3
    test_moderate_routed_llm();                       // 4
    test_ordering_safe_first_risky_last();            // 5
    test_deep_refactors_depend_on_quick_wins();       // 6
    test_skeleton_generated();                        // 7
    test_skeleton_has_modernize_comments();           // 8
    test_skeleton_annotations_collected();            // 9
    test_work_item_ids_unique();                      // 10
    test_workflow_file_metadata();                    // 11
    test_empty_source_produces_empty_workflow();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
