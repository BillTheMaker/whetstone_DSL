// Step 827: Governance and audit report bundle (8 tests)
#include "governance/GovernanceAuditReport.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

static AuditEntry makeEntry(const std::string& id, const std::string& cat) {
    AuditEntry e; e.entryId = id; e.category = cat; e.actorId = "alice";
    e.action = "approve"; e.timestamp = "2026-01-01"; e.details = "detail";
    return e;
}

void t1() {
    T(build_report);
    auto r = GovernanceAuditReportModel::build("RPT-01", {makeEntry("E1","decision")});
    C(r.totalDecisions == 1, "decisions");
    C(r.compliant, "compliant");
    P();
}
void t2() {
    T(validate_valid_report);
    auto r = GovernanceAuditReportModel::build("RPT-02", {});
    std::string err;
    C(GovernanceAuditReportModel::validate(r, &err), "valid");
    P();
}
void t3() {
    T(validate_missing_id);
    GovernanceAuditReport r;
    std::string err;
    C(!GovernanceAuditReportModel::validate(r, &err) && err == "report_id_missing", "err");
    P();
}
void t4() {
    T(category_counts);
    auto r = GovernanceAuditReportModel::build("RPT-04", {
        makeEntry("E4a","decision"), makeEntry("E4b","waiver"), makeEntry("E4c","policy")
    });
    C(r.totalDecisions == 1 && r.totalWaivers == 1 && r.totalPolicyChanges == 1, "counts");
    P();
}
void t5() {
    T(multiple_decisions);
    auto r = GovernanceAuditReportModel::build("RPT-05", {
        makeEntry("E5a","decision"), makeEntry("E5b","decision")
    });
    C(r.totalDecisions == 2, "two");
    P();
}
void t6() {
    T(empty_entries_ok);
    auto r = GovernanceAuditReportModel::build("RPT-06", {});
    C(r.entries.empty(), "empty");
    C(r.totalDecisions == 0, "zero");
    P();
}
void t7() {
    T(to_json_structure);
    auto r = GovernanceAuditReportModel::build("RPT-07", {makeEntry("E7","waiver")});
    auto j = GovernanceAuditReportModel::toJson(r);
    C(j.value("report_id","") == "RPT-07", "id");
    C(j.value("total_waivers", 0) == 1, "waivers");
    C(j.contains("entries"), "entries");
    P();
}
void t8() {
    T(compliant_flag_true);
    auto r = GovernanceAuditReportModel::build("RPT-08", {makeEntry("E8","decision")});
    C(r.compliant, "compliant");
    P();
}

int main() {
    std::cout << "Step 827: Governance and audit report bundle\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f ? 1 : 0;
}
