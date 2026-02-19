// Step 675: ContextSliceAssembler (12 tests)

#include "ContextSliceAssembler.h"

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
    fs::path dir = fs::temp_directory_path() / ("whetstone_step675_" + suffix);
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

static std::string sourceText() {
    return "int alpha() {\n"
           "  return 1;\n"
           "}\n"
           "class Demo {\n"
           "public:\n"
           "  void run() {}\n"
           "};\n"
           "int beta() {\n"
           "  return 2;\n"
           "}\n";
}

void t1() {
    TEST(empty_requests_returns_empty_result);
    WorkspaceFileIndex idx;
    auto out = ContextSliceAssembler::assemble({}, idx);
    CHECK(out.empty(), "expected empty");
    PASS();
}

void t2() {
    TEST(named_symbol_found_returns_correct_lines);
    auto dir = makeTempDir("t2");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    SliceRequest req{f.string(), "alpha", 0, 40};
    auto out = ContextSliceAssembler::assemble({req}, idx);
    CHECK(out.size() == 1, "expected one slice");
    CHECK(out[0].found, "symbol should be found");
    CHECK(out[0].content.find("int alpha()") != std::string::npos, "missing alpha");
    PASS();
}

void t3() {
    TEST(unknown_symbol_returns_head_fallback);
    auto dir = makeTempDir("t3");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "missing_symbol", 0, 2}}, idx);
    CHECK(out.size() == 1, "expected one slice");
    CHECK(!out[0].found, "found should be false");
    CHECK(out[0].content.find("int alpha()") != std::string::npos, "fallback head missing");
    PASS();
}

void t4() {
    TEST(line_range_returns_correct_lines);
    auto dir = makeTempDir("t4");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "", 2, 2}}, idx);
    CHECK(out.size() == 1, "expected one slice");
    CHECK(out[0].lineStart == 2 && out[0].lineEnd == 3, "line range mismatch");
    PASS();
}

void t5() {
    TEST(content_non_empty_on_success);
    auto dir = makeTempDir("t5");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "beta", 0, 40}}, idx);
    CHECK(!out.empty(), "slice missing");
    CHECK(!out[0].content.empty(), "content empty");
    PASS();
}

void t6() {
    TEST(found_false_when_symbol_missing);
    auto dir = makeTempDir("t6");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "zzz", 0, 1}}, idx);
    CHECK(out.size() == 1, "slice missing");
    CHECK(!out[0].found, "should be false");
    PASS();
}

void t7() {
    TEST(multiple_requests_all_processed);
    auto dir = makeTempDir("t7");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    std::vector<SliceRequest> reqs = {
        {f.string(), "alpha", 0, 40},
        {f.string(), "beta", 0, 40},
        {f.string(), "", 1, 1}
    };
    auto out = ContextSliceAssembler::assemble(reqs, idx);
    CHECK(out.size() == 3, "expected 3 slices");
    PASS();
}

void t8() {
    TEST(head_fallback_respects_head_lines);
    auto dir = makeTempDir("t8");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "none", 0, 1}}, idx);
    CHECK(out.size() == 1, "slice missing");
    CHECK(out[0].lineEnd == 1, "headLines not respected");
    PASS();
}

void t9() {
    TEST(function_body_slice_includes_closing_brace);
    auto dir = makeTempDir("t9");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "alpha", 0, 40}}, idx);
    CHECK(out.size() == 1, "slice missing");
    CHECK(out[0].content.find("}") != std::string::npos, "closing brace not included");
    PASS();
}

void t10() {
    TEST(class_slice_includes_closing_brace);
    auto dir = makeTempDir("t10");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "Demo", 0, 40}}, idx);
    CHECK(out.size() == 1, "slice missing");
    CHECK(out[0].content.find("};") != std::string::npos, "class close not included");
    PASS();
}

void t11() {
    TEST(file_path_preserved_in_result);
    auto dir = makeTempDir("t11");
    auto f = dir / "demo.cpp";
    writeFile(f, sourceText());
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "alpha", 0, 40}}, idx);
    CHECK(out.size() == 1, "slice missing");
    CHECK(out[0].filePath == f.string(), "path not preserved");
    PASS();
}

void t12() {
    TEST(slice_content_is_substring_of_source);
    auto dir = makeTempDir("t12");
    auto f = dir / "demo.cpp";
    const std::string src = sourceText();
    writeFile(f, src);
    auto idx = WorkspaceFileIndex::build(dir.string());
    auto out = ContextSliceAssembler::assemble({{f.string(), "beta", 0, 40}}, idx);
    CHECK(out.size() == 1, "slice missing");
    CHECK(src.find(out[0].content.substr(0, std::min<size_t>(10, out[0].content.size()))) != std::string::npos,
          "slice content not from source");
    PASS();
}

int main() {
    std::cout << "Step 675: ContextSliceAssembler\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

