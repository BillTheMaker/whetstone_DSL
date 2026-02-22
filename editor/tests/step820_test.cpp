// Step 820: Ambiguity triage UI model and API (10 tests)
#include "governance/AmbiguityTriageModel.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

static AmbiguityItem makeItem(const std::string& id, AmbiguitySeverity sev = AmbiguitySeverity::Medium) {
    AmbiguityItem item; item.itemId = id; item.description = "desc " + id; item.severity = sev; return item;
}

void t1() {
    T(add_item_success);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A1"), &err), "add");
    C(m.summarize().total == 1, "total");
    P();
}
void t2() {
    T(prevent_duplicate);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A2"), &err), "first");
    C(!m.addItem(makeItem("A2"), &err) && err == "item_duplicate", "dup");
    P();
}
void t3() {
    T(missing_description);
    AmbiguityTriageModel m; std::string err;
    AmbiguityItem bad; bad.itemId = "A3";
    C(!m.addItem(bad, &err) && err == "description_missing", "no_desc");
    P();
}
void t4() {
    T(assign_reviewer);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A4"), &err), "add");
    C(m.assign("A4", "alice", &err), "assign");
    C(m.items()[0].status == AmbiguityStatus::UnderReview, "status");
    P();
}
void t5() {
    T(assign_empty_reviewer);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A5"), &err), "add");
    C(!m.assign("A5", "", &err) && err == "reviewer_missing", "err");
    P();
}
void t6() {
    T(resolve_item);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A6"), &err), "add");
    C(m.assign("A6", "bob", &err), "assign");
    C(m.resolve("A6", "use override pattern", &err), "resolve");
    C(m.summarize().resolved == 1, "resolved");
    P();
}
void t7() {
    T(resolve_requires_assignment);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A7"), &err), "add");
    C(!m.resolve("A7", "some fix", &err) && err == "not_assigned", "err");
    P();
}
void t8() {
    T(defer_item);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A8"), &err), "add");
    C(m.defer("A8", &err), "defer");
    C(m.summarize().deferred == 1, "deferred");
    P();
}
void t9() {
    T(high_severity_open_flag);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A9", AmbiguitySeverity::High), &err), "add");
    C(m.summarize().hasHighSeverityOpen, "high");
    P();
}
void t10() {
    T(summary_multi_status);
    AmbiguityTriageModel m; std::string err;
    C(m.addItem(makeItem("A10"), &err), "add1");
    C(m.addItem(makeItem("A11"), &err), "add2");
    C(m.assign("A10", "carol", &err), "assign");
    auto s = m.summarize();
    C(s.open == 1 && s.underReview == 1, "counts");
    P();
}

int main() {
    std::cout << "Step 820: Ambiguity triage UI model and API\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f ? 1 : 0;
}
