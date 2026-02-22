// Step 822: Reviewer decision ledger integration (10 tests)
#include "governance/ReviewerDecisionLedger.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

static LedgerEntry makeEntry(const std::string& id, const std::string& decision = "approved") {
    LedgerEntry e; e.entryId = id; e.issueRef = "ISS-01"; e.reviewer = "alice";
    e.decision = decision; e.rationale = "looks good"; return e;
}

void t1() {
    T(append_entry);
    ReviewerDecisionLedger l; std::string err;
    C(l.append(makeEntry("E1"), &err), "append");
    C(l.entries().size() == 1, "size");
    P();
}
void t2() {
    T(missing_entry_id);
    ReviewerDecisionLedger l; std::string err;
    LedgerEntry e; e.issueRef="X"; e.reviewer="alice"; e.decision="approved"; e.rationale="ok";
    C(!l.append(e, &err) && err == "entry_id_missing", "err");
    P();
}
void t3() {
    T(missing_reviewer);
    ReviewerDecisionLedger l; std::string err;
    LedgerEntry e; e.entryId="E3"; e.issueRef="X"; e.decision="approved"; e.rationale="ok";
    C(!l.append(e, &err) && err == "reviewer_missing", "err");
    P();
}
void t4() {
    T(missing_rationale);
    ReviewerDecisionLedger l; std::string err;
    LedgerEntry e; e.entryId="E4"; e.issueRef="X"; e.reviewer="alice"; e.decision="approved";
    C(!l.append(e, &err) && err == "rationale_missing", "err");
    P();
}
void t5() {
    T(summarize_decisions);
    ReviewerDecisionLedger l; std::string err;
    l.append(makeEntry("E5a", "approved"), &err);
    l.append(makeEntry("E5b", "rejected"), &err);
    l.append(makeEntry("E5c", "deferred"), &err);
    auto s = l.summarize();
    C(s.total == 3 && s.approved == 1 && s.rejected == 1 && s.deferred == 1, "counts");
    P();
}
void t6() {
    T(has_waivers_false);
    ReviewerDecisionLedger l; std::string err;
    l.append(makeEntry("E6"), &err);
    C(!l.summarize().hasWaivers, "no_waiver");
    P();
}
void t7() {
    T(has_waivers_true);
    ReviewerDecisionLedger l; std::string err;
    auto e = makeEntry("E7"); e.waiverRef = "waiver:temporary";
    l.append(e, &err);
    C(l.summarize().hasWaivers, "waiver");
    P();
}
void t8() {
    T(for_issue_filter);
    ReviewerDecisionLedger l; std::string err;
    LedgerEntry e1 = makeEntry("E8a"); e1.issueRef = "ISS-A";
    LedgerEntry e2 = makeEntry("E8b"); e2.issueRef = "ISS-B";
    l.append(e1, &err); l.append(e2, &err);
    auto r = l.forIssue("ISS-A");
    C(r.size() == 1 && r[0].entryId == "E8a", "filter");
    P();
}
void t9() {
    T(modified_count);
    ReviewerDecisionLedger l; std::string err;
    l.append(makeEntry("E9", "modified"), &err);
    C(l.summarize().modified == 1, "modified");
    P();
}
void t10() {
    T(to_json_summary);
    ReviewerDecisionLedger l; std::string err;
    l.append(makeEntry("E10"), &err);
    auto j = ReviewerDecisionLedger::toJson(l.summarize());
    C(j.value("total", 0) == 1, "total");
    C(j.value("approved", 0) == 1, "approved");
    P();
}

int main() {
    std::cout << "Step 822: Reviewer decision ledger integration\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f ? 1 : 0;
}
