// Step 680: SelfContainmentScorer (12 tests)

#include "SelfContainmentScorer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static fs::path makeRoot(const std::string& suffix) {
    fs::path root = fs::temp_directory_path() / ("whetstone_step680_" + suffix);
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "editor/src", ec);
    std::ofstream(root / "editor/src/file.h") << "int x();\n";
    return root;
}

static TaskitemInput makeGood(const std::string& root) {
    TaskitemInput in;
    in.taskId = "T1";
    in.title = "Implement feature";
    in.prerequisiteOps = {"read editor/src/file.h"};
    in.reasons = {"reason1", "reason2", "reason3"};
    in.confidence = 90;
    (void)root;
    return in;
}

void t1() {
    TEST(fully_specified_scores_high);
    auto root = makeRoot("t1");
    auto s = SelfContainmentScorer::score(makeGood(root.string()), root.string());
    CHECK(s.score >= 80, "expected >= 80");
    PASS();
}

void t2() {
    TEST(empty_prerequisite_ops_deducts_30);
    auto root = makeRoot("t2");
    auto in = makeGood(root.string());
    in.prerequisiteOps.clear();
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(s.score == 70, "expected 70");
    PASS();
}

void t3() {
    TEST(missing_reasons_deducts_20);
    auto root = makeRoot("t3");
    auto in = makeGood(root.string());
    in.reasons.clear();
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(s.score == 80, "expected 80");
    PASS();
}

void t4() {
    TEST(empty_title_deducts_20);
    auto root = makeRoot("t4");
    auto in = makeGood(root.string());
    in.title.clear();
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(s.score == 80, "expected 80");
    PASS();
}

void t5() {
    TEST(unresolved_file_deducts_10);
    auto root = makeRoot("t5");
    auto in = makeGood(root.string());
    in.prerequisiteOps = {"read editor/src/missing.h"};
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(s.score == 90, "expected 90");
    PASS();
}

void t6() {
    TEST(two_unresolved_files_deduct_20);
    auto root = makeRoot("t6");
    auto in = makeGood(root.string());
    in.prerequisiteOps = {"read editor/src/m1.h", "read editor/src/m2.h"};
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(s.score == 80, "expected 80");
    PASS();
}

void t7() {
    TEST(score_clamped_to_zero);
    auto root = makeRoot("t7");
    TaskitemInput in;
    in.taskId = "T2";
    in.confidence = 0;
    in.prerequisiteOps = {};
    in.reasons = {};
    in.title = "";
    in.dependencyTaskIds = {"A", "B"};
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(s.score >= 0, "score below 0");
    PASS();
}

void t8() {
    TEST(score_clamped_to_100);
    auto root = makeRoot("t8");
    auto s = SelfContainmentScorer::score(makeGood(root.string()), root.string());
    CHECK(s.score <= 100, "score above 100");
    PASS();
}

void t9() {
    TEST(self_contained_true_at_80_or_more);
    auto root = makeRoot("t9");
    auto in = makeGood(root.string());
    in.reasons.clear(); // -20 => 80
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(s.selfContained, "expected selfContained true");
    PASS();
}

void t10() {
    TEST(self_contained_false_below_80);
    auto root = makeRoot("t10");
    auto in = makeGood(root.string());
    in.prerequisiteOps.clear(); // 70
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(!s.selfContained, "expected selfContained false");
    PASS();
}

void t11() {
    TEST(issues_non_empty_when_deductions);
    auto root = makeRoot("t11");
    auto in = makeGood(root.string());
    in.title.clear();
    auto s = SelfContainmentScorer::score(in, root.string());
    CHECK(!s.issues.empty(), "issues should be non-empty");
    PASS();
}

void t12() {
    TEST(issues_empty_when_perfect);
    auto root = makeRoot("t12");
    auto s = SelfContainmentScorer::score(makeGood(root.string()), root.string());
    CHECK(s.issues.empty(), "issues should be empty");
    PASS();
}

int main() {
    std::cout << "Step 680: SelfContainmentScorer\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

