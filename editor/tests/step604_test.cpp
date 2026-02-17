// Step 604: Release Certification Packet (12 tests)

#include "ReleaseCertificationPacket.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static CertificationCheck check(const std::string& id,
                                const std::string& category,
                                bool passed = false) {
    return {id, category, passed, "evidence/" + id};
}

void test_record_check_success() {
    TEST(record_check_success);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(packet.recordCheck(check("c1", "security"), &error), "record should succeed");
    CHECK(packet.failedCount() == 1, "new check defaults to failed");
    PASS();
}

void test_record_check_rejects_missing_check_id() {
    TEST(record_check_rejects_missing_check_id);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(!packet.recordCheck(check("", "security"), &error), "record should fail");
    CHECK(error == "check_id_missing", "wrong error");
    PASS();
}

void test_record_check_rejects_missing_category() {
    TEST(record_check_rejects_missing_category);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(!packet.recordCheck(check("c1", ""), &error), "record should fail");
    CHECK(error == "category_missing", "wrong error");
    PASS();
}

void test_record_check_rejects_missing_evidence_ref() {
    TEST(record_check_rejects_missing_evidence_ref);
    ReleaseCertificationPacket packet;
    std::string error;
    auto c = check("c1", "security");
    c.evidenceRef.clear();
    CHECK(!packet.recordCheck(c, &error), "record should fail");
    CHECK(error == "evidence_ref_missing", "wrong error");
    PASS();
}

void test_record_check_rejects_duplicate() {
    TEST(record_check_rejects_duplicate);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(packet.recordCheck(check("c1", "security"), &error), "first record failed");
    CHECK(!packet.recordCheck(check("c1", "quality"), &error), "duplicate should fail");
    CHECK(error == "check_duplicate", "wrong error");
    PASS();
}

void test_set_result_success() {
    TEST(set_result_success);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(packet.recordCheck(check("c1", "security"), &error), "record failed");
    CHECK(packet.setResult("c1", true, &error), "set result should succeed");
    CHECK(packet.passedCount() == 1, "passed count mismatch");
    PASS();
}

void test_set_result_rejects_missing_check() {
    TEST(set_result_rejects_missing_check);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(!packet.setResult("missing", true, &error), "set result should fail");
    CHECK(error == "check_missing", "wrong error");
    PASS();
}

void test_failed_count_tracks_unpassed_checks() {
    TEST(failed_count_tracks_unpassed_checks);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(packet.recordCheck(check("c1", "security"), &error), "record c1 failed");
    CHECK(packet.recordCheck(check("c2", "quality"), &error), "record c2 failed");
    CHECK(packet.setResult("c2", true, &error), "set result c2 failed");
    CHECK(packet.failedCount() == 1, "failed count mismatch");
    PASS();
}

void test_all_passed_false_when_empty() {
    TEST(all_passed_false_when_empty);
    ReleaseCertificationPacket packet;
    CHECK(!packet.allPassed(), "empty packet should not pass");
    PASS();
}

void test_all_passed_true_when_every_check_passes() {
    TEST(all_passed_true_when_every_check_passes);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(packet.recordCheck(check("c1", "security"), &error), "record c1 failed");
    CHECK(packet.recordCheck(check("c2", "quality"), &error), "record c2 failed");
    CHECK(packet.setResult("c1", true, &error), "set result c1 failed");
    CHECK(packet.setResult("c2", true, &error), "set result c2 failed");
    CHECK(packet.allPassed(), "all checks should pass");
    PASS();
}

void test_by_category_filters() {
    TEST(by_category_filters);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(packet.recordCheck(check("c1", "security"), &error), "record c1 failed");
    CHECK(packet.recordCheck(check("c2", "quality"), &error), "record c2 failed");
    CHECK(packet.byCategory("security").size() == 1, "category filter mismatch");
    PASS();
}

void test_by_category_empty_returns_all() {
    TEST(by_category_empty_returns_all);
    ReleaseCertificationPacket packet;
    std::string error;
    CHECK(packet.recordCheck(check("c1", "security"), &error), "record c1 failed");
    CHECK(packet.recordCheck(check("c2", "quality"), &error), "record c2 failed");
    CHECK(packet.byCategory("").size() == 2, "empty category should return all");
    PASS();
}

int main() {
    std::cout << "Step 604: Release Certification Packet\n";

    test_record_check_success();                       // 1
    test_record_check_rejects_missing_check_id();     // 2
    test_record_check_rejects_missing_category();     // 3
    test_record_check_rejects_missing_evidence_ref(); // 4
    test_record_check_rejects_duplicate();            // 5
    test_set_result_success();                        // 6
    test_set_result_rejects_missing_check();          // 7
    test_failed_count_tracks_unpassed_checks();       // 8
    test_all_passed_false_when_empty();               // 9
    test_all_passed_true_when_every_check_passes();   // 10
    test_by_category_filters();                       // 11
    test_by_category_empty_returns_all();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
