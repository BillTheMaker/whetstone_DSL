#pragma once
// Step 532: Retry/Escalation Protocol for Constraint Failures

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ConstraintViolationDiagnostics.h"

using json = nlohmann::json;

struct RetryAttempt {
    int attemptNumber = 0;
    bool success = false;
    std::vector<std::string> appliedFixes;
    ConstraintDiagnosticPacket diagnostics;
};

struct RetryEscalationInput {
    ConstraintDiagnosticPacket initialDiagnostics;
    int retryBudget = 2;
};

struct RetryEscalationResult {
    bool resolved = false;
    bool escalated = false;
    int attemptsUsed = 0;
    std::vector<RetryAttempt> attempts;
    ConstraintDiagnosticPacket finalDiagnostics;
    json escalationPacket;
};

class RetryEscalationProtocol {
public:
    static RetryEscalationResult run(const RetryEscalationInput& input) {
        RetryEscalationResult result;
        result.finalDiagnostics = input.initialDiagnostics;

        if (input.initialDiagnostics.ok) {
            result.resolved = true;
            return result;
        }

        if (hasNonRetryable(input.initialDiagnostics)) {
            result.escalated = true;
            result.escalationPacket = makeEscalationPacket(input.initialDiagnostics,
                                                           result.attempts,
                                                           "non_retryable_violation");
            return result;
        }

        int budget = input.retryBudget < 0 ? 0 : input.retryBudget;
        ConstraintDiagnosticPacket current = input.initialDiagnostics;

        for (int attemptNo = 1; attemptNo <= budget; ++attemptNo) {
            RetryAttempt attempt;
            attempt.attemptNumber = attemptNo;
            attempt.diagnostics = applyDeterministicRepair(current, attempt.appliedFixes);
            attempt.success = attempt.diagnostics.ok;
            result.attempts.push_back(attempt);
            result.attemptsUsed = attemptNo;
            current = attempt.diagnostics;

            if (current.ok) {
                result.resolved = true;
                result.finalDiagnostics = current;
                return result;
            }
        }

        result.finalDiagnostics = current;
        result.escalated = true;
        result.escalationPacket = makeEscalationPacket(current,
                                                       result.attempts,
                                                       "retry_budget_exhausted");
        return result;
    }

private:
    static bool hasNonRetryable(const ConstraintDiagnosticPacket& packet) {
        for (const auto& v : packet.violations) {
            if (!v.retryable) return true;
        }
        return false;
    }

    static ConstraintDiagnosticPacket applyDeterministicRepair(
        const ConstraintDiagnosticPacket& current,
        std::vector<std::string>& appliedFixes) {
        ConstraintDiagnosticPacket next;
        for (const auto& violation : current.violations) {
            if (isAutoCorrectable(violation)) {
                appliedFixes.push_back("drop-symbol:" + violation.field);
                continue;
            }
            next.violations.push_back(violation);
        }

        next.ok = next.violations.empty();
        next.recommendedAction = next.ok ? "proceed"
                                         : (hasNonRetryable(next) ? "escalate" : "retry");
        return next;
    }

    static bool isAutoCorrectable(const ConstraintViolation& violation) {
        return violation.retryable &&
               (violation.code == "out_of_scope_symbol" ||
                violation.code == "transient_scope_miss");
    }

    static json makeEscalationPacket(const ConstraintDiagnosticPacket& finalDiag,
                                     const std::vector<RetryAttempt>& attempts,
                                     const std::string& reason) {
        json packet;
        packet["reason"] = reason;
        packet["finalDiagnostics"] = finalDiag.toJson();
        packet["attempts"] = json::array();
        for (const auto& a : attempts) {
            packet["attempts"].push_back({
                {"attemptNumber", a.attemptNumber},
                {"success", a.success},
                {"appliedFixes", a.appliedFixes},
                {"diagnostics", a.diagnostics.toJson()}
            });
        }
        return packet;
    }
};
