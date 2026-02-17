// Step 610: Service Dependency Risk Map (12 tests)

#include "ServiceDependencyRiskMap.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static DependencyEdge edge(const std::string& id,
                           const std::string& src,
                           const std::string& dst,
                           DependencyRisk risk = DependencyRisk::Low) {
    return {id, src, dst, risk};
}

void test_add_edge_success() {
    TEST(add_edge_success);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(map.addEdge(edge("e1", "svc-a", "svc-b"), &error), "add should succeed");
    CHECK(map.byService("svc-a").size() == 1, "service count mismatch");
    PASS();
}

void test_add_edge_rejects_missing_edge_id() {
    TEST(add_edge_rejects_missing_edge_id);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(!map.addEdge(edge("", "svc-a", "svc-b"), &error), "add should fail");
    CHECK(error == "edge_id_missing", "wrong error");
    PASS();
}

void test_add_edge_rejects_missing_source() {
    TEST(add_edge_rejects_missing_source);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(!map.addEdge(edge("e1", "", "svc-b"), &error), "add should fail");
    CHECK(error == "source_service_missing", "wrong error");
    PASS();
}

void test_add_edge_rejects_missing_target() {
    TEST(add_edge_rejects_missing_target);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(!map.addEdge(edge("e1", "svc-a", ""), &error), "add should fail");
    CHECK(error == "target_service_missing", "wrong error");
    PASS();
}

void test_add_edge_rejects_duplicate() {
    TEST(add_edge_rejects_duplicate);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(map.addEdge(edge("e1", "svc-a", "svc-b"), &error), "first add failed");
    CHECK(!map.addEdge(edge("e1", "svc-a", "svc-c"), &error), "duplicate should fail");
    CHECK(error == "edge_duplicate", "wrong error");
    PASS();
}

void test_update_risk_success() {
    TEST(update_risk_success);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(map.addEdge(edge("e1", "svc-a", "svc-b"), &error), "add failed");
    CHECK(map.updateRisk("e1", DependencyRisk::High, &error), "update should succeed");
    CHECK(map.highRiskOutboundCount("svc-a") == 1, "outbound count mismatch");
    PASS();
}

void test_update_risk_rejects_missing_edge() {
    TEST(update_risk_rejects_missing_edge);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(!map.updateRisk("missing", DependencyRisk::High, &error), "update should fail");
    CHECK(error == "edge_missing", "wrong error");
    PASS();
}

void test_high_risk_inbound_count() {
    TEST(high_risk_inbound_count);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(map.addEdge(edge("e1", "svc-a", "svc-b", DependencyRisk::High), &error), "add e1 failed");
    CHECK(map.addEdge(edge("e2", "svc-c", "svc-b", DependencyRisk::Medium), &error), "add e2 failed");
    CHECK(map.addEdge(edge("e3", "svc-d", "svc-b", DependencyRisk::High), &error), "add e3 failed");
    CHECK(map.highRiskInboundCount("svc-b") == 2, "inbound high-risk count mismatch");
    PASS();
}

void test_high_risk_outbound_count() {
    TEST(high_risk_outbound_count);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(map.addEdge(edge("e1", "svc-a", "svc-b", DependencyRisk::High), &error), "add e1 failed");
    CHECK(map.addEdge(edge("e2", "svc-a", "svc-c", DependencyRisk::Low), &error), "add e2 failed");
    CHECK(map.addEdge(edge("e3", "svc-a", "svc-d", DependencyRisk::High), &error), "add e3 failed");
    CHECK(map.highRiskOutboundCount("svc-a") == 2, "outbound high-risk count mismatch");
    PASS();
}

void test_by_service_filters_source_or_target() {
    TEST(by_service_filters_source_or_target);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(map.addEdge(edge("e1", "svc-a", "svc-b"), &error), "add e1 failed");
    CHECK(map.addEdge(edge("e2", "svc-c", "svc-a"), &error), "add e2 failed");
    CHECK(map.addEdge(edge("e3", "svc-x", "svc-y"), &error), "add e3 failed");
    CHECK(map.byService("svc-a").size() == 2, "service filter mismatch");
    PASS();
}

void test_by_service_empty_returns_all() {
    TEST(by_service_empty_returns_all);
    ServiceDependencyRiskMap map;
    std::string error;
    CHECK(map.addEdge(edge("e1", "svc-a", "svc-b"), &error), "add e1 failed");
    CHECK(map.addEdge(edge("e2", "svc-c", "svc-d"), &error), "add e2 failed");
    CHECK(map.byService("").size() == 2, "empty filter should return all");
    PASS();
}

void test_counts_zero_for_unknown_service() {
    TEST(counts_zero_for_unknown_service);
    ServiceDependencyRiskMap map;
    CHECK(map.highRiskInboundCount("missing") == 0, "inbound unknown should be zero");
    CHECK(map.highRiskOutboundCount("missing") == 0, "outbound unknown should be zero");
    PASS();
}

int main() {
    std::cout << "Step 610: Service Dependency Risk Map\n";

    test_add_edge_success();                          // 1
    test_add_edge_rejects_missing_edge_id();         // 2
    test_add_edge_rejects_missing_source();          // 3
    test_add_edge_rejects_missing_target();          // 4
    test_add_edge_rejects_duplicate();               // 5
    test_update_risk_success();                      // 6
    test_update_risk_rejects_missing_edge();         // 7
    test_high_risk_inbound_count();                  // 8
    test_high_risk_outbound_count();                 // 9
    test_by_service_filters_source_or_target();      // 10
    test_by_service_empty_returns_all();             // 11
    test_counts_zero_for_unknown_service();          // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
