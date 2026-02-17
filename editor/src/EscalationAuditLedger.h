#pragma once
// Step 596: Escalation Audit Ledger

#include "ApprovalEscalationPlanner.h"

#include <map>
#include <string>
#include <vector>

struct EscalationAuditEntry {
    std::string entryId;
    std::string operation;
    GuardrailDecision decision = GuardrailDecision::RequireReview;
    EscalationLevel level = EscalationLevel::Reviewer;
    std::string actor;
    std::string outcome;
    int riskScore = 0;
};

class EscalationAuditLedger {
public:
    bool record(const EscalationAuditEntry& entry, std::string* error) {
        if (!error) return false;
        error->clear();
        if (entry.entryId.empty()) return fail(error, "entry_id_missing");
        if (entry.operation.empty()) return fail(error, "operation_missing");
        if (entry.actor.empty()) return fail(error, "actor_missing");
        if (entry.outcome.empty()) return fail(error, "outcome_missing");
        if (entry.riskScore < 0 || entry.riskScore > 100) return fail(error, "risk_score_invalid");
        if (entriesById_.count(entry.entryId) != 0) return fail(error, "entry_duplicate");
        entriesById_[entry.entryId] = entry;
        order_.push_back(entry.entryId);
        return true;
    }

    std::vector<EscalationAuditEntry> entries() const {
        std::vector<EscalationAuditEntry> out;
        for (const auto& id : order_) out.push_back(entriesById_.at(id));
        return out;
    }

    std::vector<EscalationAuditEntry> byActor(const std::string& actor) const {
        std::vector<EscalationAuditEntry> out;
        for (const auto& id : order_) {
            const auto& e = entriesById_.at(id);
            if (actor.empty() || e.actor == actor) out.push_back(e);
        }
        return out;
    }

    int averageRiskScore() const {
        if (order_.empty()) return 0;
        int sum = 0;
        for (const auto& id : order_) sum += entriesById_.at(id).riskScore;
        return sum / static_cast<int>(order_.size());
    }

    int deniedCount() const {
        int count = 0;
        for (const auto& id : order_) {
            if (entriesById_.at(id).decision == GuardrailDecision::Deny) ++count;
        }
        return count;
    }

private:
    std::map<std::string, EscalationAuditEntry> entriesById_;
    std::vector<std::string> order_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }
};
