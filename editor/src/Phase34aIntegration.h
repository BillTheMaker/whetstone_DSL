#pragma once
// Step 598: Phase 34a Integration

#include "ApprovalEscalationPlanner.h"
#include "EscalationAuditLedger.h"
#include "GuardrailDriftMonitor.h"
#include "PolicyGuardrailCatalog.h"

#include <string>
#include <vector>

struct Phase34aResult {
    bool guardrailCatalogReady = false;
    bool escalationPlannerReady = false;
    bool auditLedgerReady = false;
    bool driftMonitorReady = false;
    bool phase34aPass = false;
    int driftScore = 0;
    std::vector<std::string> notes;
};

class Phase34aIntegration {
public:
    static Phase34aResult run() {
        Phase34aResult r;
        std::string error;

        PolicyGuardrailCatalog catalog;
        r.guardrailCatalogReady =
            catalog.addRule({"allow-push", "git push", GuardrailDecision::Allow, "safe push"}, &error) &&
            catalog.addRule({"review-deploy", "deploy", GuardrailDecision::RequireReview, "deploy review"}, &error) &&
            catalog.addRule({"deny-rm", "rm -rf", GuardrailDecision::Deny, "destructive"}, &error);
        if (!r.guardrailCatalogReady) {
            r.notes.push_back("fail:guardrail_catalog");
            return finalize(r);
        }

        auto plan = ApprovalEscalationPlanner::planFor(catalog, "deploy prod");
        r.escalationPlannerReady = plan.level == EscalationLevel::Reviewer &&
                                   plan.requiresJustification &&
                                   !plan.approverRoles.empty();
        if (!r.escalationPlannerReady) {
            r.notes.push_back("fail:escalation_planner");
            return finalize(r);
        }

        EscalationAuditLedger ledger;
        EscalationAuditEntry entry;
        entry.entryId = "audit-1";
        entry.operation = "deploy prod";
        entry.decision = plan.decision;
        entry.level = plan.level;
        entry.actor = "architect";
        entry.outcome = "approved";
        entry.riskScore = ApprovalEscalationPlanner::riskScore(plan);
        r.auditLedgerReady = ledger.record(entry, &error) &&
                             ledger.averageRiskScore() > 0;
        if (!r.auditLedgerReady) {
            r.notes.push_back("fail:audit_ledger");
            return finalize(r);
        }

        std::vector<DriftObservation> observations = {
            {"git push origin", "allow"},
            {"deploy prod", "require_review"},
            {"rm -rf /tmp/x", "allow"}
        };
        auto drift = GuardrailDriftMonitor::detect(catalog, observations);
        r.driftScore = GuardrailDriftMonitor::driftScore(drift, static_cast<int>(observations.size()));
        r.driftMonitorReady = !drift.empty() && r.driftScore > 0 &&
                              !GuardrailDriftMonitor::remediationHints(drift).empty();
        if (!r.driftMonitorReady) {
            r.notes.push_back("fail:drift_monitor");
            return finalize(r);
        }

        r.notes.push_back("phase34a:policy_governance_cycle_ready");
        return finalize(r);
    }

private:
    static Phase34aResult finalize(Phase34aResult r) {
        r.phase34aPass = r.guardrailCatalogReady &&
                         r.escalationPlannerReady &&
                         r.auditLedgerReady &&
                         r.driftMonitorReady;
        if (!r.phase34aPass) r.notes.push_back("phase34a:blocked");
        return r;
    }
};
