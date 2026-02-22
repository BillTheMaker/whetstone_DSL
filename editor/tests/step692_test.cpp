// Step 692: migration acceptance contract (10 tests)

#include "MigrationAcceptanceContract.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static MigrationEvidence goodEvidence() {
    MigrationEvidence e;
    e.buildSuccess = true;
    e.testsProvided = true;
    e.testPassRate = 1.0f;
    e.securityReportProvided = true;
    e.highSeverityFindings = 0;
    e.benchmarkProvided = true;
    e.perfRegressionPct = 3.0f;
    e.checklistComplete = true;
    return e;
}

void t1() {
    TEST(pass_path);
    auto out = MigrationAcceptanceContract::evaluate(goodEvidence(), MigrationGateThresholds{});
    CHECK(out.pass, "expected pass");
    PASS();
}

void t2() {
    TEST(single_gate_fail);
    auto e = goodEvidence();
    e.buildSuccess = false;
    auto out = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{});
    CHECK(!out.pass, "expected fail");
    CHECK(out.failedGates.size() == 1, "expected one failed gate");
    PASS();
}

void t3() {
    TEST(multiple_gate_fail);
    MigrationEvidence e;
    auto out = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{});
    CHECK(!out.pass, "expected fail");
    CHECK(out.failedGates.size() >= 3, "expected multiple failed gates");
    PASS();
}

void t4() {
    TEST(threshold_edge_cases);
    auto e = goodEvidence();
    MigrationGateThresholds th;
    th.maxPerfRegressionPct = 3.0f;
    auto out = MigrationAcceptanceContract::evaluate(e, th);
    CHECK(out.pass, "edge threshold should pass");
    PASS();
}

void t5() {
    TEST(missing_benchmark_data);
    auto e = goodEvidence();
    e.benchmarkProvided = false;
    auto out = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{});
    CHECK(!out.pass, "expected fail");
    CHECK(out.gateStates.value("benchmark_data", "") == "missing", "wrong benchmark state");
    PASS();
}

void t6() {
    TEST(missing_security_report);
    auto e = goodEvidence();
    e.securityReportProvided = false;
    auto out = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{});
    CHECK(!out.pass, "expected fail");
    CHECK(out.gateStates.value("security_report", "") == "missing", "wrong security state");
    PASS();
}

void t7() {
    TEST(missing_tests);
    auto e = goodEvidence();
    e.testsProvided = false;
    auto out = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{});
    CHECK(!out.pass, "expected fail");
    CHECK(out.gateStates.value("tests_provided", "") == "missing", "wrong tests state");
    PASS();
}

void t8() {
    TEST(deterministic_result_packet);
    auto e = goodEvidence();
    std::string a = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{}).gateStates.dump();
    std::string b = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{}).gateStates.dump();
    CHECK(a == b, "nondeterministic packet");
    PASS();
}

void t9() {
    TEST(machine_readable_reason_codes);
    auto e = goodEvidence();
    e.perfRegressionPct = 99.0f;
    auto out = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{});
    CHECK(out.gateStates.value("performance_regression", "") == "above_threshold", "missing reason code");
    PASS();
}

void t10() {
    TEST(human_readable_summary_text);
    auto e = goodEvidence();
    auto out = MigrationAcceptanceContract::evaluate(e, MigrationGateThresholds{});
    CHECK(out.summary == "all_gates_passed", "summary mismatch");
    PASS();
}

int main() {
    std::cout << "Step 692: migration acceptance contract\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
