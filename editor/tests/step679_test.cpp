// Step 679: PrerequisiteOpResolver (12 tests)

#include "PrerequisiteOpResolver.h"

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
    fs::path root = fs::temp_directory_path() / ("whetstone_step679_" + suffix);
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "editor/src", ec);
    std::ofstream(root / "editor/src/file.h") << "int x();\n";
    return root;
}

void t1() {
    TEST(empty_op_string_unknown);
    auto r = PrerequisiteOpResolver::resolve("", ".");
    CHECK(r.kind == OpKind::Unknown, "expected unknown");
    PASS();
}

void t2() {
    TEST(read_op_kind_readfile);
    auto root = makeRoot("t2");
    auto r = PrerequisiteOpResolver::resolve("read editor/src/file.h", root.string());
    CHECK(r.kind == OpKind::ReadFile, "expected read kind");
    PASS();
}

void t3() {
    TEST(read_op_file_path_non_empty);
    auto root = makeRoot("t3");
    auto r = PrerequisiteOpResolver::resolve("read editor/src/file.h", root.string());
    CHECK(!r.filePath.empty(), "filePath should be non-empty");
    PASS();
}

void t4() {
    TEST(file_exists_true_when_present);
    auto root = makeRoot("t4");
    auto r = PrerequisiteOpResolver::resolve("read editor/src/file.h", root.string());
    CHECK(r.fileExists, "file should exist");
    PASS();
}

void t5() {
    TEST(file_exists_false_when_absent);
    auto root = makeRoot("t5");
    auto r = PrerequisiteOpResolver::resolve("read editor/src/missing.h", root.string());
    CHECK(!r.fileExists, "file should be absent");
    PASS();
}

void t6() {
    TEST(line_range_parsed);
    auto root = makeRoot("t6");
    auto r = PrerequisiteOpResolver::resolve("read editor/src/file.h lines 30-60", root.string());
    CHECK(r.lineStart == 30 && r.lineEnd == 60, "line range parse failed");
    PASS();
}

void t7() {
    TEST(no_line_range_defaults_zero);
    auto root = makeRoot("t7");
    auto r = PrerequisiteOpResolver::resolve("read editor/src/file.h", root.string());
    CHECK(r.lineStart == 0 && r.lineEnd == 0, "lines should default 0");
    PASS();
}

void t8() {
    TEST(run_op_kind_runcommand);
    auto r = PrerequisiteOpResolver::resolve("run cmake --build editor/build-native", ".");
    CHECK(r.kind == OpKind::RunCommand, "expected run command");
    PASS();
}

void t9() {
    TEST(run_command_has_empty_file_path);
    auto r = PrerequisiteOpResolver::resolve("run cmake --build editor/build-native", ".");
    CHECK(r.filePath.empty(), "run command should have empty filePath");
    PASS();
}

void t10() {
    TEST(resolve_all_multiple_ops);
    auto root = makeRoot("t10");
    std::vector<std::string> ops = {"read editor/src/file.h", "run cmake --build ."};
    auto out = PrerequisiteOpResolver::resolveAll(ops, root.string());
    CHECK(out.size() == 2, "expected two entries");
    PASS();
}

void t11() {
    TEST(raw_field_preserved);
    const std::string raw = "read editor/src/file.h lines 1-2";
    auto r = PrerequisiteOpResolver::resolve(raw, ".");
    CHECK(r.raw == raw, "raw should be preserved");
    PASS();
}

void t12() {
    TEST(unknown_op_graceful);
    auto r = PrerequisiteOpResolver::resolve("inspect editor/src/file.h", ".");
    CHECK(r.kind == OpKind::Unknown, "should be unknown");
    PASS();
}

int main() {
    std::cout << "Step 679: PrerequisiteOpResolver\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

