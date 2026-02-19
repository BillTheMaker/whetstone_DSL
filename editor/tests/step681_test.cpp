// Step 681: TaskitemQualityAuditor (12 tests)

#include "TaskitemQualityAuditor.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static fs::path makeRoot(const std::string& suffix) {
    fs::path root = fs::temp_directory_path() / ("whetstone_step681_" + suffix);
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "editor/src", ec);
    std::ofstream(root / "editor/src/file.h") << "int x();\n";
    return root;
}

static TaskitemInput good() {
    TaskitemInput in;
    in.taskId = "T";
    in.title = "Title";
    in.prerequisiteOps = {"read editor/src/file.h"};
    in.reasons = {"a", "b", "c"};
    in.confidence = 80;
    return in;
}

void t1() {
    TEST(empty_input_valid_zero_report);
    auto r = TaskitemQualityAuditor::audit({}, ".");
    CHECK(r.totalTaskitems == 0, "expected 0 total");
    CHECK(r.results.empty(), "results should be empty");
    PASS();
}

void t2() {
    TEST(single_passing_item_counted);
    auto root = makeRoot("t2");
    auto r = TaskitemQualityAuditor::audit({good()}, root.string());
    CHECK(r.selfContainedCount == 1, "expected one passing");
    PASS();
}

void t3() {
    TEST(single_failing_item_counted);
    auto root = makeRoot("t3");
    TaskitemInput bad;
    bad.taskId = "B";
    auto r = TaskitemQualityAuditor::audit({bad}, root.string());
    CHECK(r.failingCount == 1, "expected one failing");
    PASS();
}

void t4() {
    TEST(average_score_computed);
    auto root = makeRoot("t4");
    auto g = good();
    TaskitemInput b;
    b.taskId = "B";
    auto r = TaskitemQualityAuditor::audit({g, b}, root.string());
    CHECK(r.averageScore >= 0.0, "average should be >=0");
    CHECK(r.averageScore <= 100.0, "average should be <=100");
    PASS();
}

void t5() {
    TEST(top_issues_non_empty_when_issues_exist);
    auto root = makeRoot("t5");
    TaskitemInput b;
    b.taskId = "B";
    auto r = TaskitemQualityAuditor::audit({b}, root.string());
    CHECK(!r.topIssues.empty(), "topIssues should be non-empty");
    PASS();
}

void t6() {
    TEST(category_counts_sum_to_total);
    auto root = makeRoot("t6");
    auto g = good();
    TaskitemInput b;
    b.taskId = "B";
    auto r = TaskitemQualityAuditor::audit({g, b}, root.string());
    CHECK(r.selfContainedCount + r.warningCount + r.failingCount == r.totalTaskitems,
          "count sum mismatch");
    PASS();
}

void t7() {
    TEST(results_size_matches_input_size);
    auto root = makeRoot("t7");
    auto g = good();
    TaskitemInput b;
    b.taskId = "B";
    auto r = TaskitemQualityAuditor::audit({g, b}, root.string());
    CHECK(r.results.size() == 2, "results size mismatch");
    PASS();
}

void t8() {
    TEST(all_results_have_task_id);
    auto root = makeRoot("t8");
    auto g = good();
    auto r = TaskitemQualityAuditor::audit({g}, root.string());
    CHECK(!r.results[0].taskId.empty(), "task id missing");
    PASS();
}

void t9() {
    TEST(mixed_batch_counted_correctly);
    auto root = makeRoot("t9");
    auto g = good();
    TaskitemInput w = good();
    w.taskId = "W";
    w.prerequisiteOps.clear(); // 70 warning
    TaskitemInput b;
    b.taskId = "B";
    auto r = TaskitemQualityAuditor::audit({g, w, b}, root.string());
    CHECK(r.selfContainedCount == 1, "expected 1 self-contained");
    CHECK(r.warningCount == 1, "expected 1 warning");
    CHECK(r.failingCount == 1, "expected 1 failing");
    PASS();
}

void t10() {
    TEST(report_deterministic);
    auto root = makeRoot("t10");
    auto g = good();
    TaskitemInput b;
    b.taskId = "B";
    auto r1 = TaskitemQualityAuditor::audit({g, b}, root.string());
    auto r2 = TaskitemQualityAuditor::audit({g, b}, root.string());
    CHECK(r1.averageScore == r2.averageScore, "non-deterministic average");
    CHECK(r1.topIssues == r2.topIssues, "non-deterministic issues");
    PASS();
}

void t11() {
    TEST(total_taskitems_matches_input);
    auto root = makeRoot("t11");
    auto g = good();
    auto r = TaskitemQualityAuditor::audit({g, g, g}, root.string());
    CHECK(r.totalTaskitems == 3, "expected total 3");
    PASS();
}

void t12() {
    TEST(warning_count_correct_for_range);
    auto root = makeRoot("t12");
    auto w = good();
    w.prerequisiteOps.clear(); // 70
    auto r = TaskitemQualityAuditor::audit({w}, root.string());
    CHECK(r.warningCount == 1, "expected one warning");
    PASS();
}

int main() {
    std::cout << "Step 681: TaskitemQualityAuditor\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

