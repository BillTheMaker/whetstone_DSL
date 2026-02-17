// Step 602: Control Attestation Registry (12 tests)

#include "ControlAttestationRegistry.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ControlAttestation attestation(const std::string& id,
                                      const std::string& control,
                                      int days) {
    return {id, control, "owner", days, AttestationStatus::Draft, ""};
}

void test_submit_success() {
    TEST(submit_success);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(registry.submit(attestation("a1", "ctrl-a", 30), &error), "submit should succeed");
    CHECK(registry.byStatus(AttestationStatus::Draft).size() == 1, "draft count mismatch");
    PASS();
}

void test_submit_rejects_missing_attestation_id() {
    TEST(submit_rejects_missing_attestation_id);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(!registry.submit(attestation("", "ctrl-a", 30), &error), "submit should fail");
    CHECK(error == "attestation_id_missing", "wrong error");
    PASS();
}

void test_submit_rejects_missing_control_id() {
    TEST(submit_rejects_missing_control_id);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(!registry.submit(attestation("a1", "", 30), &error), "submit should fail");
    CHECK(error == "control_id_missing", "wrong error");
    PASS();
}

void test_submit_rejects_missing_owner() {
    TEST(submit_rejects_missing_owner);
    ControlAttestationRegistry registry;
    std::string error;
    auto a = attestation("a1", "ctrl-a", 30);
    a.owner.clear();
    CHECK(!registry.submit(a, &error), "submit should fail");
    CHECK(error == "owner_missing", "wrong error");
    PASS();
}

void test_submit_rejects_negative_expiry_days() {
    TEST(submit_rejects_negative_expiry_days);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(!registry.submit(attestation("a1", "ctrl-a", -1), &error), "submit should fail");
    CHECK(error == "expiry_days_invalid", "wrong error");
    PASS();
}

void test_submit_rejects_expired_status() {
    TEST(submit_rejects_expired_status);
    ControlAttestationRegistry registry;
    std::string error;
    auto a = attestation("a1", "ctrl-a", 30);
    a.status = AttestationStatus::Expired;
    CHECK(!registry.submit(a, &error), "submit should fail");
    CHECK(error == "status_invalid", "wrong error");
    PASS();
}

void test_submit_rejects_duplicate() {
    TEST(submit_rejects_duplicate);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(registry.submit(attestation("a1", "ctrl-a", 30), &error), "first submit failed");
    CHECK(!registry.submit(attestation("a1", "ctrl-b", 10), &error), "duplicate should fail");
    CHECK(error == "attestation_duplicate", "wrong error");
    PASS();
}

void test_sign_success_sets_signed_status() {
    TEST(sign_success_sets_signed_status);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(registry.submit(attestation("a1", "ctrl-a", 30), &error), "submit failed");
    CHECK(registry.sign("a1", "reviewer", &error), "sign should succeed");
    CHECK(registry.byStatus(AttestationStatus::Signed).size() == 1, "signed count mismatch");
    PASS();
}

void test_sign_rejects_missing_attestation() {
    TEST(sign_rejects_missing_attestation);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(!registry.sign("missing", "reviewer", &error), "sign should fail");
    CHECK(error == "attestation_missing", "wrong error");
    PASS();
}

void test_sign_rejects_missing_signer() {
    TEST(sign_rejects_missing_signer);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(registry.submit(attestation("a1", "ctrl-a", 30), &error), "submit failed");
    CHECK(!registry.sign("a1", "", &error), "sign should fail");
    CHECK(error == "signer_missing", "wrong error");
    PASS();
}

void test_expire_then_sign_fails() {
    TEST(expire_then_sign_fails);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(registry.submit(attestation("a1", "ctrl-a", 30), &error), "submit failed");
    CHECK(registry.expire("a1", &error), "expire failed");
    CHECK(!registry.sign("a1", "reviewer", &error), "sign should fail");
    CHECK(error == "attestation_expired", "wrong error");
    PASS();
}

void test_expiring_within_ignores_expired_items() {
    TEST(expiring_within_ignores_expired_items);
    ControlAttestationRegistry registry;
    std::string error;
    CHECK(registry.submit(attestation("a1", "ctrl-a", 5), &error), "submit a1 failed");
    CHECK(registry.submit(attestation("a2", "ctrl-a", 20), &error), "submit a2 failed");
    CHECK(registry.submit(attestation("a3", "ctrl-b", 3), &error), "submit a3 failed");
    CHECK(registry.expire("a3", &error), "expire a3 failed");
    CHECK(registry.expiringWithin(10) == 1, "expiring count mismatch");
    PASS();
}

int main() {
    std::cout << "Step 602: Control Attestation Registry\n";

    test_submit_success();                            // 1
    test_submit_rejects_missing_attestation_id();    // 2
    test_submit_rejects_missing_control_id();        // 3
    test_submit_rejects_missing_owner();             // 4
    test_submit_rejects_negative_expiry_days();      // 5
    test_submit_rejects_expired_status();            // 6
    test_submit_rejects_duplicate();                 // 7
    test_sign_success_sets_signed_status();          // 8
    test_sign_rejects_missing_attestation();         // 9
    test_sign_rejects_missing_signer();              // 10
    test_expire_then_sign_fails();                   // 11
    test_expiring_within_ignores_expired_items();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
