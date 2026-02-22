// Step 691: capability matrix model (10 tests)

#include "LanguageCapabilityMatrix.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static LanguageCapabilityRow rustRow() {
    return {
        "rust", "systems", "static", "borrow-checked", "result+panic",
        "async+threads", "macros", "ffi-unsafe-boundary", "cargo",
        "native runtime", false
    };
}

void t1() {
    TEST(add_read_capability_rows);
    std::vector<LanguageCapabilityRow> rows;
    LanguageCapabilityMatrix::addOrUpdate(&rows, rustRow());
    const auto* row = LanguageCapabilityMatrix::get(rows, "rust");
    CHECK(row != nullptr, "row missing");
    CHECK(row->typeModel == "static", "row content mismatch");
    PASS();
}

void t2() {
    TEST(paradigm_grouping);
    auto rows = LanguageCapabilityMatrix::defaultRows();
    auto grouped = LanguageCapabilityMatrix::groupByParadigm(rows);
    CHECK(grouped.find("systems") != grouped.end(), "systems group missing");
    CHECK(!grouped["systems"].empty(), "systems group empty");
    PASS();
}

void t3() {
    TEST(missing_fields_flagged);
    LanguageCapabilityRow bad;
    bad.language = "";
    std::string err;
    CHECK(!LanguageCapabilityMatrix::validateRow(bad, false, &err), "expected failure");
    CHECK(err == "language_missing", "wrong error");
    PASS();
}

void t4() {
    TEST(versioned_capability_snapshots);
    std::vector<LanguageCapabilityRow> v1{rustRow()};
    std::vector<LanguageCapabilityRow> v2{rustRow()};
    v2[0].macroModel = "macros+proc";
    auto diffs = LanguageCapabilityMatrix::diff(v1, v2);
    CHECK(diffs.size() == 1, "expected single diff");
    CHECK(diffs[0].language == "rust", "wrong language diffed");
    PASS();
}

void t5() {
    TEST(diff_capability_versions);
    std::vector<LanguageCapabilityRow> oldRows{rustRow()};
    std::vector<LanguageCapabilityRow> newRows{rustRow(), {"zig", "systems", "static", "manual", "error unions", "async", "comptime", "c abi", "zig build", "native", true}};
    auto diffs = LanguageCapabilityMatrix::diff(oldRows, newRows);
    CHECK(diffs.size() == 1, "expected one added row diff");
    CHECK(diffs[0].changedFields.size() == 1, "expected row_added marker");
    CHECK(diffs[0].changedFields[0] == "row_added", "missing row_added marker");
    PASS();
}

void t6() {
    TEST(deterministic_output);
    auto rows = LanguageCapabilityMatrix::defaultRows();
    std::string a = LanguageCapabilityMatrix::toJson(rows).dump();
    std::string b = LanguageCapabilityMatrix::toJson(rows).dump();
    CHECK(a == b, "nondeterministic output");
    PASS();
}

void t7() {
    TEST(unknown_language_allowed_experimental);
    LanguageCapabilityRow row;
    row.language = "mydsl";
    std::string err;
    CHECK(LanguageCapabilityMatrix::validateRow(row, false, &err), "should allow in non-strict");
    PASS();
}

void t8() {
    TEST(strict_mode_failure);
    LanguageCapabilityRow row;
    row.language = "mydsl";
    std::string err;
    CHECK(!LanguageCapabilityMatrix::validateRow(row, true, &err), "expected strict failure");
    CHECK(err == "paradigm_missing", "wrong strict error");
    PASS();
}

void t9() {
    TEST(export_to_json);
    auto rows = LanguageCapabilityMatrix::defaultRows();
    auto j = LanguageCapabilityMatrix::toJson(rows);
    CHECK(j.is_array(), "json export should be array");
    CHECK(!j.empty(), "json export empty");
    PASS();
}

void t10() {
    TEST(import_from_json);
    auto rows = LanguageCapabilityMatrix::defaultRows();
    auto j = LanguageCapabilityMatrix::toJson(rows);
    auto parsed = LanguageCapabilityMatrix::fromJson(j);
    CHECK(parsed.size() == rows.size(), "import count mismatch");
    CHECK(parsed[0].language <= parsed.back().language, "expected sorted export/import ordering");
    PASS();
}

int main() {
    std::cout << "Step 691: capability matrix model\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
