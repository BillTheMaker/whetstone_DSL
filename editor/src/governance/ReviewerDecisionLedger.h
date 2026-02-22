#pragma once
// Step 822: Reviewer decision ledger integration.

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct LedgerEntry {
    std::string entryId;
    std::string issueRef;
    std::string reviewer;
    std::string decision;  // "approved", "rejected", "deferred", "modified"
    std::string rationale;
    std::string timestamp;
    std::string waiverRef;
};

struct LedgerSummary {
    int total = 0;
    int approved = 0;
    int rejected = 0;
    int deferred = 0;
    int modified = 0;
    bool hasWaivers = false;
};

class ReviewerDecisionLedger {
public:
    bool append(const LedgerEntry& entry, std::string* error) {
        if (!error) return false;
        error->clear();
        if (entry.entryId.empty()) return fail(error, "entry_id_missing");
        if (entry.issueRef.empty()) return fail(error, "issue_ref_missing");
        if (entry.reviewer.empty()) return fail(error, "reviewer_missing");
        if (entry.decision.empty()) return fail(error, "decision_missing");
        if (entry.rationale.empty()) return fail(error, "rationale_missing");
        entries_.push_back(entry);
        return true;
    }

    std::vector<LedgerEntry> forIssue(const std::string& issueRef) const {
        std::vector<LedgerEntry> out;
        for (const auto& e : entries_) if (e.issueRef == issueRef) out.push_back(e);
        return out;
    }

    LedgerSummary summarize() const {
        LedgerSummary s;
        for (const auto& e : entries_) {
            ++s.total;
            if (!e.waiverRef.empty()) s.hasWaivers = true;
            if (e.decision == "approved") ++s.approved;
            else if (e.decision == "rejected") ++s.rejected;
            else if (e.decision == "deferred") ++s.deferred;
            else if (e.decision == "modified") ++s.modified;
        }
        return s;
    }

    const std::vector<LedgerEntry>& entries() const { return entries_; }

    static nlohmann::json toJson(const LedgerSummary& s) {
        return {{"total", s.total}, {"approved", s.approved}, {"rejected", s.rejected},
                {"deferred", s.deferred}, {"modified", s.modified}, {"has_waivers", s.hasWaivers}};
    }

    static nlohmann::json entryToJson(const LedgerEntry& e) {
        return {{"entry_id", e.entryId}, {"issue_ref", e.issueRef}, {"reviewer", e.reviewer},
                {"decision", e.decision}, {"rationale", e.rationale},
                {"timestamp", e.timestamp}, {"waiver_ref", e.waiverRef}};
    }

private:
    std::vector<LedgerEntry> entries_;
    static bool fail(std::string* e, const char* code) { *e = code; return false; }
};
