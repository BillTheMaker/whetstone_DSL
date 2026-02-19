// Step 674: WorkspaceFileIndex (12 tests)

#include "WorkspaceFileIndex.h"

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

static fs::path makeTempDir(const std::string& suffix) {
    fs::path dir = fs::temp_directory_path() / ("whetstone_step674_" + suffix);
    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);
    return dir;
}

static void writeFile(const fs::path& p, const std::string& content) {
    std::error_code ec;
    fs::create_directories(p.parent_path(), ec);
    std::ofstream out(p);
    out << content;
}

void t1() {
    TEST(empty_root_returns_empty_index);
    auto idx = WorkspaceFileIndex::build("/tmp/whetstone_step674_does_not_exist");
    CHECK(idx.fileCount() == 0, "file count should be 0");
    CHECK(idx.symbolCount() == 0, "symbol count should be 0");
    PASS();
}

void t2() {
    TEST(single_file_workspace_indexes_it);
    auto dir = makeTempDir("t2");
    writeFile(dir / "a.cpp", "int foo() { return 1; }\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    CHECK(idx.fileCount() == 1, "expected one file");
    PASS();
}

void t3() {
    TEST(function_signatures_found_by_name);
    auto dir = makeTempDir("t3");
    writeFile(dir / "a.cpp", "int add(int a, int b) { return a + b; }\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto locs = idx.find("add");
    CHECK(!locs.empty(), "function add not found");
    PASS();
}

void t4() {
    TEST(class_declarations_found_by_name);
    auto dir = makeTempDir("t4");
    writeFile(dir / "a.h", "class Engine { public: void run(); };\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto locs = idx.find("Engine");
    CHECK(!locs.empty(), "class Engine not found");
    PASS();
}

void t5() {
    TEST(struct_declarations_found_by_name);
    auto dir = makeTempDir("t5");
    writeFile(dir / "a.h", "struct Payload { int x; };\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto locs = idx.find("Payload");
    CHECK(!locs.empty(), "struct Payload not found");
    PASS();
}

void t6() {
    TEST(find_in_file_returns_symbols_for_file);
    auto dir = makeTempDir("t6");
    writeFile(dir / "src/main.cpp", "int one() { return 1; }\nint two() { return 2; }\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto locs = idx.findInFile("src/main.cpp");
    CHECK(locs.size() >= 2, "expected at least 2 symbols");
    PASS();
}

void t7() {
    TEST(find_returns_matches_across_files);
    auto dir = makeTempDir("t7");
    writeFile(dir / "a.cpp", "int same() { return 1; }\n");
    writeFile(dir / "b.cpp", "int same() { return 2; }\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto locs = idx.find("same");
    CHECK(locs.size() == 2, "expected 2 matches");
    PASS();
}

void t8() {
    TEST(symbol_count_gt_zero_after_indexing);
    auto dir = makeTempDir("t8");
    writeFile(dir / "a.cpp", "int abc() { return 3; }\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    CHECK(idx.symbolCount() > 0, "symbolCount should be > 0");
    PASS();
}

void t9() {
    TEST(file_count_matches_input_file_count);
    auto dir = makeTempDir("t9");
    writeFile(dir / "a.cpp", "int a() { return 1; }\n");
    writeFile(dir / "b.cpp", "int b() { return 2; }\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    CHECK(idx.fileCount() == 2, "expected fileCount 2");
    PASS();
}

void t10() {
    TEST(duplicate_symbol_names_all_returned);
    auto dir = makeTempDir("t10");
    writeFile(dir / "x1.cpp", "int dup() { return 1; }\n");
    writeFile(dir / "x2.cpp", "int dup() { return 2; }\n");
    writeFile(dir / "x3.cpp", "int dup() { return 3; }\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    CHECK(idx.find("dup").size() == 3, "expected 3 duplicates");
    PASS();
}

void t11() {
    TEST(unreadable_or_nonexistent_root_skips_gracefully);
    auto idx = WorkspaceFileIndex::build("");
    CHECK(idx.fileCount() == 0, "should gracefully return empty");
    PASS();
}

void t12() {
    TEST(kind_field_is_non_empty);
    auto dir = makeTempDir("t12");
    writeFile(dir / "a.cpp", "using SizeT = unsigned long;\n");
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto locs = idx.find("SizeT");
    CHECK(!locs.empty(), "symbol missing");
    CHECK(!locs.front().symbolKind.empty(), "symbol kind empty");
    PASS();
}

int main() {
    std::cout << "Step 674: WorkspaceFileIndex\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

