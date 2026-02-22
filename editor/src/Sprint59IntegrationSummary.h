#pragma once
// Step 828: Sprint 59 integration summary.

#include <string>
#include <vector>
#include <algorithm>

#include "governance/PortingReviewBoard.h"
#include "governance/AmbiguityTriageModel.h"
#include "governance/WaiverPolicyPacket.h"
#include "governance/ReviewerDecisionLedger.h"
#include "governance/PolicyPackSystem.h"
#include "governance/DecisionReplayChecker.h"
#include "governance/FeedbackAdapterHooks.h"
#include "governance/GovernanceAuditReport.h"
#include "MCPServer.h"

struct Sprint59IntegrationResult {
    bool reviewBoardReady = false;
    bool ambiguityTriageReady = false;
    bool waiverPacketReady = false;
    bool ledgerReady = false;
    bool policyPackReady = false;
    bool replayCheckerReady = false;
    bool feedbackHooksReady = false;
    bool auditReportReady = false;
    bool reviewToolReady = false;
    bool success = false;
    int stepStart = 820;
    int stepEnd = 828;
    std::vector<std::string> filesAdded;
};

class Sprint59IntegrationSummary {
public:
    static Sprint59IntegrationResult run() {
        Sprint59IntegrationResult out;
        out.filesAdded = {
            "governance/AmbiguityTriageModel.h",
            "governance/WaiverPolicyPacket.h",
            "governance/ReviewerDecisionLedger.h",
            "governance/PolicyPackSystem.h",
            "governance/DecisionReplayChecker.h",
            "governance/FeedbackAdapterHooks.h",
            "governance/GovernanceAuditReport.h",
            "mcp/RegisterGovernanceTools.h",
            "Sprint59IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        PortingReviewBoard board;
        PortingReviewIssue issue;
        issue.issueId = "ISS-59";
        issue.description = "Sprint 59 integration check";
        std::string err;
        out.reviewBoardReady = board.addIssue(issue, &err);

        AmbiguityTriageModel triage;
        AmbiguityItem item;
        item.itemId = "AMB-01";
        item.description = "Test ambiguity";
        out.ambiguityTriageReady = triage.addItem(item, &err);

        auto waiver = WaiverPolicyPacketModel::make("W-01", "ISS-59",
            WaiverScope::Temporary, "test rationale", "alice");
        waiver.expiresAt = "2026-12-31";
        out.waiverPacketReady = WaiverPolicyPacketModel::validate(waiver, &err);

        ReviewerDecisionLedger ledger;
        LedgerEntry le;
        le.entryId = "LE-01"; le.issueRef = "ISS-59";
        le.reviewer = "alice"; le.decision = "approved"; le.rationale = "ok";
        out.ledgerReady = ledger.append(le, &err);

        auto pack = PolicyPackSystem::buildSafeFirst();
        out.policyPackReady = PolicyPackSystem::validate(pack, &err);

        DecisionSnapshot snap;
        snap.snapshotId = "SN-01"; snap.issueRef = "ISS-59";
        snap.policy = "safe-first"; snap.reviewer = "alice";
        snap.decision = "approved"; snap.rationale = "ok";
        auto replay = DecisionReplayChecker::replay(snap, snap);
        out.replayCheckerReady = replay.fullyReproducible;

        FeedbackAdapterHooks hooks;
        FeedbackSignal sig;
        sig.signalId = "SG-01"; sig.adapterTarget = "cpp"; sig.suggestion = "use smart ptrs"; sig.weight = 5;
        out.feedbackHooksReady = hooks.addSignal(sig, &err);

        auto report = GovernanceAuditReportModel::build("RPT-59", {});
        out.auditReportReady = !report.reportId.empty();

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_review_porting_decision") out.reviewToolReady = true;
        }

        out.success = out.reviewBoardReady && out.ambiguityTriageReady && out.waiverPacketReady &&
                      out.ledgerReady && out.policyPackReady && out.replayCheckerReady &&
                      out.feedbackHooksReady && out.auditReportReady && out.reviewToolReady &&
                      out.stepStart == 820 && out.stepEnd == 828;
        return out;
    }
};
