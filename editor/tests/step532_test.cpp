// Step 532: Retry/Escalation Protocol (12 tests)

#include "RetryEscalationProtocol.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ConstraintViolation mkViolation(const std::string& code,
                                       const std::string& field,
                                       bool retryable) {
    return ConstraintViolation{code, code + " message", field, retryable};
}

static ConstraintDiagnosticPacket mkPacket(
    const std::vector<ConstraintViolation>& violations) {
    ConstraintDiagnosticPacket p;
    p.violations = violations;
    p.ok = violations.empty();
    p.recommendedAction = p.ok ? "proceed" : "escalate";
    return p;
}

void test_ok_packet_resolves_without_retry() {
    TEST(ok_packet_resolves_without_retry);
    RetryEscalationInput input{mkPacket({}), 2};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(result.resolved, "ok packet should resolve");
    CHECK(!result.escalated, "ok packet should not escalate");
    CHECK(result.attemptsUsed == 0, "no attempts should be used");
    PASS();
}

void test_non_retryable_violation_escalates_immediately() {
    TEST(non_retryable_violation_escalates_immediately);
    RetryEscalationInput input{mkPacket({mkViolation("illegal_operation", "delete", false)}), 3};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(!result.resolved, "non-retryable should not resolve");
    CHECK(result.escalated, "non-retryable should escalate");
    CHECK(result.attemptsUsed == 0, "should not spend retry attempts");
    CHECK(result.escalationPacket["reason"] == "non_retryable_violation", "wrong escalation reason");
    PASS();
}

void test_retryable_out_of_scope_resolves_in_one_retry() {
    TEST(retryable_out_of_scope_resolves_in_one_retry);
    RetryEscalationInput input{mkPacket({mkViolation("out_of_scope_symbol", "ghost", true)}), 2};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(result.resolved, "retryable out-of-scope should resolve");
    CHECK(!result.escalated, "resolved packet should not escalate");
    CHECK(result.attemptsUsed == 1, "should resolve on first retry");
    PASS();
}

void test_mixed_retryable_and_non_retryable_escalates_without_retry() {
    TEST(mixed_retryable_and_non_retryable_escalates_without_retry);
    RetryEscalationInput input{mkPacket({
        mkViolation("out_of_scope_symbol", "ghost", true),
        mkViolation("forbidden_symbol", "unsafeGlobal", false)
    }), 2};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(result.escalated, "mixed packet should escalate");
    CHECK(result.attemptsUsed == 0, "mixed packet should not retry");
    PASS();
}

void test_retry_budget_exhaustion_escalates() {
    TEST(retry_budget_exhaustion_escalates);
    RetryEscalationInput input{mkPacket({mkViolation("retryable_unknown", "x", true)}), 2};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(!result.resolved, "unknown retryable should remain unresolved");
    CHECK(result.escalated, "budget exhaustion should escalate");
    CHECK(result.attemptsUsed == 2, "should consume full retry budget");
    CHECK(result.escalationPacket["reason"] == "retry_budget_exhausted", "wrong exhaustion reason");
    PASS();
}

void test_escalation_packet_contains_attempt_history() {
    TEST(escalation_packet_contains_attempt_history);
    RetryEscalationInput input{mkPacket({mkViolation("retryable_unknown", "x", true)}), 1};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(result.escalationPacket.contains("attempts"), "packet should contain attempts");
    CHECK(result.escalationPacket["attempts"].size() == 1, "attempt count should match retries");
    PASS();
}

void test_protocol_is_deterministic_for_same_input() {
    TEST(protocol_is_deterministic_for_same_input);
    RetryEscalationInput input{mkPacket({mkViolation("retryable_unknown", "x", true)}), 2};
    auto a = RetryEscalationProtocol::run(input);
    auto b = RetryEscalationProtocol::run(input);
    CHECK(a.attemptsUsed == b.attemptsUsed, "attempt counts should match");
    CHECK(a.escalated == b.escalated, "escalation decision should be deterministic");
    CHECK(a.finalDiagnostics.violations.size() == b.finalDiagnostics.violations.size(),
          "final diagnostics should match");
    PASS();
}

void test_multiple_correctable_retryables_resolve_together() {
    TEST(multiple_correctable_retryables_resolve_together);
    RetryEscalationInput input{mkPacket({
        mkViolation("out_of_scope_symbol", "ghostA", true),
        mkViolation("transient_scope_miss", "ghostB", true)
    }), 2};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(result.resolved, "correctable retryables should resolve");
    CHECK(result.attemptsUsed == 1, "should resolve in one deterministic pass");
    PASS();
}

void test_final_diagnostics_retry_when_unresolved_retryable() {
    TEST(final_diagnostics_retry_when_unresolved_retryable);
    RetryEscalationInput input{mkPacket({mkViolation("retryable_unknown", "x", true)}), 1};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(!result.finalDiagnostics.ok, "final diagnostics should remain failing");
    CHECK(result.finalDiagnostics.recommendedAction == "retry", "final diagnostics should request retry");
    PASS();
}

void test_zero_retry_budget_escalates_for_retryable_failure() {
    TEST(zero_retry_budget_escalates_for_retryable_failure);
    RetryEscalationInput input{mkPacket({mkViolation("out_of_scope_symbol", "ghost", true)}), 0};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(result.escalated, "zero budget should escalate unresolved failure");
    CHECK(result.attemptsUsed == 0, "no retries should be used");
    PASS();
}

void test_attempt_records_applied_fixes_for_correctable_failure() {
    TEST(attempt_records_applied_fixes_for_correctable_failure);
    RetryEscalationInput input{mkPacket({mkViolation("out_of_scope_symbol", "ghost", true)}), 2};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(!result.attempts.empty(), "attempt history expected");
    CHECK(result.attempts[0].appliedFixes.size() == 1, "one fix should be recorded");
    CHECK(result.attempts[0].appliedFixes[0] == "drop-symbol:ghost", "fix metadata mismatch");
    PASS();
}

void test_escalation_packet_contains_final_diagnostics() {
    TEST(escalation_packet_contains_final_diagnostics);
    RetryEscalationInput input{mkPacket({mkViolation("retryable_unknown", "x", true)}), 1};
    auto result = RetryEscalationProtocol::run(input);
    CHECK(result.escalationPacket.contains("finalDiagnostics"), "packet should include final diagnostics");
    CHECK(result.escalationPacket["finalDiagnostics"]["ok"] == false,
          "final diagnostics in packet should indicate failure");
    PASS();
}

int main() {
    std::cout << "Step 532: Retry/Escalation Protocol\n";

    test_ok_packet_resolves_without_retry();                          // 1
    test_non_retryable_violation_escalates_immediately();             // 2
    test_retryable_out_of_scope_resolves_in_one_retry();              // 3
    test_mixed_retryable_and_non_retryable_escalates_without_retry(); // 4
    test_retry_budget_exhaustion_escalates();                         // 5
    test_escalation_packet_contains_attempt_history();                // 6
    test_protocol_is_deterministic_for_same_input();                  // 7
    test_multiple_correctable_retryables_resolve_together();          // 8
    test_final_diagnostics_retry_when_unresolved_retryable();         // 9
    test_zero_retry_budget_escalates_for_retryable_failure();         // 10
    test_attempt_records_applied_fixes_for_correctable_failure();     // 11
    test_escalation_packet_contains_final_diagnostics();              // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
