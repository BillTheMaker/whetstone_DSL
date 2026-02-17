// Step 607: Operational SLO Watch (12 tests)

#include "OperationalSLOWatch.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SLOIndicator indicator(const std::string& id,
                              const std::string& service,
                              int target,
                              int observed,
                              int burn) {
    return {id, service, target, observed, burn};
}

void test_upsert_success() {
    TEST(upsert_success);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(watch.upsert(indicator("i1", "svc-a", 99, 99, 50), &error), "upsert should succeed");
    CHECK(watch.byService("svc-a").size() == 1, "service count mismatch");
    PASS();
}

void test_upsert_rejects_missing_indicator_id() {
    TEST(upsert_rejects_missing_indicator_id);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(!watch.upsert(indicator("", "svc-a", 99, 99, 50), &error), "upsert should fail");
    CHECK(error == "indicator_id_missing", "wrong error");
    PASS();
}

void test_upsert_rejects_missing_service_id() {
    TEST(upsert_rejects_missing_service_id);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(!watch.upsert(indicator("i1", "", 99, 99, 50), &error), "upsert should fail");
    CHECK(error == "service_id_missing", "wrong error");
    PASS();
}

void test_upsert_rejects_invalid_target_percent() {
    TEST(upsert_rejects_invalid_target_percent);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(!watch.upsert(indicator("i1", "svc-a", 120, 99, 50), &error), "upsert should fail");
    CHECK(error == "target_percent_invalid", "wrong error");
    PASS();
}

void test_upsert_rejects_invalid_observed_percent() {
    TEST(upsert_rejects_invalid_observed_percent);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(!watch.upsert(indicator("i1", "svc-a", 99, -1, 50), &error), "upsert should fail");
    CHECK(error == "observed_percent_invalid", "wrong error");
    PASS();
}

void test_upsert_rejects_negative_error_budget_burn() {
    TEST(upsert_rejects_negative_error_budget_burn);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(!watch.upsert(indicator("i1", "svc-a", 99, 99, -1), &error), "upsert should fail");
    CHECK(error == "error_budget_burn_invalid", "wrong error");
    PASS();
}

void test_breaching_true_when_observed_below_target() {
    TEST(breaching_true_when_observed_below_target);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(watch.upsert(indicator("i1", "svc-a", 99, 98, 20), &error), "upsert failed");
    CHECK(watch.breaching("i1", &error), "indicator should breach");
    PASS();
}

void test_breaching_true_when_burn_exceeds_hundred() {
    TEST(breaching_true_when_burn_exceeds_hundred);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(watch.upsert(indicator("i1", "svc-a", 99, 100, 101), &error), "upsert failed");
    CHECK(watch.breaching("i1", &error), "indicator should breach");
    PASS();
}

void test_breaching_false_when_healthy() {
    TEST(breaching_false_when_healthy);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(watch.upsert(indicator("i1", "svc-a", 99, 100, 40), &error), "upsert failed");
    CHECK(!watch.breaching("i1", &error), "indicator should be healthy");
    PASS();
}

void test_breaching_rejects_missing_indicator() {
    TEST(breaching_rejects_missing_indicator);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(!watch.breaching("missing", &error), "breach should fail");
    CHECK(error == "indicator_missing", "wrong error");
    PASS();
}

void test_service_breach_count_counts_only_breaches() {
    TEST(service_breach_count_counts_only_breaches);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(watch.upsert(indicator("i1", "svc-a", 99, 98, 10), &error), "upsert i1 failed");
    CHECK(watch.upsert(indicator("i2", "svc-a", 99, 100, 10), &error), "upsert i2 failed");
    CHECK(watch.upsert(indicator("i3", "svc-a", 99, 100, 110), &error), "upsert i3 failed");
    CHECK(watch.serviceBreachCount("svc-a") == 2, "breach count mismatch");
    PASS();
}

void test_by_service_empty_returns_all() {
    TEST(by_service_empty_returns_all);
    OperationalSLOWatch watch;
    std::string error;
    CHECK(watch.upsert(indicator("i1", "svc-a", 99, 99, 10), &error), "upsert i1 failed");
    CHECK(watch.upsert(indicator("i2", "svc-b", 99, 99, 10), &error), "upsert i2 failed");
    CHECK(watch.byService("").size() == 2, "empty service should return all");
    PASS();
}

int main() {
    std::cout << "Step 607: Operational SLO Watch\n";

    test_upsert_success();                               // 1
    test_upsert_rejects_missing_indicator_id();         // 2
    test_upsert_rejects_missing_service_id();           // 3
    test_upsert_rejects_invalid_target_percent();       // 4
    test_upsert_rejects_invalid_observed_percent();     // 5
    test_upsert_rejects_negative_error_budget_burn();   // 6
    test_breaching_true_when_observed_below_target();   // 7
    test_breaching_true_when_burn_exceeds_hundred();    // 8
    test_breaching_false_when_healthy();                // 9
    test_breaching_rejects_missing_indicator();         // 10
    test_service_breach_count_counts_only_breaches();   // 11
    test_by_service_empty_returns_all();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
