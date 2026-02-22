// Step 824: Decision replay and reproducibility checker (10 tests)
#include "governance/DecisionReplayChecker.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

static DecisionSnapshot makeSnap(const std::string& id, const std::string& decision = "approved") {
    DecisionSnapshot s; s.snapshotId = id; s.issueRef = "ISS-01";
    s.policy = "safe-first"; s.reviewer = "alice"; s.decision = decision; s.rationale = "ok";
    return s;
}

void t1() {
    T(identical_snapshots_reproducible);
    auto s = makeSnap("SN-01");
    auto r = DecisionReplayChecker::replay(s, s);
    C(r.fullyReproducible, "repro");
    P();
}
void t2() {
    T(policy_mismatch);
    auto a = makeSnap("SN-02"); auto b = makeSnap("SN-02");
    b.policy = "interop-first";
    auto r = DecisionReplayChecker::replay(a, b);
    C(!r.fullyReproducible && r.divergenceReason == "policy_changed", "policy");
    P();
}
void t3() {
    T(reviewer_mismatch);
    auto a = makeSnap("SN-03"); auto b = makeSnap("SN-03");
    b.reviewer = "bob";
    auto r = DecisionReplayChecker::replay(a, b);
    C(!r.fullyReproducible && r.divergenceReason == "reviewer_changed", "reviewer");
    P();
}
void t4() {
    T(decision_mismatch);
    auto a = makeSnap("SN-04", "approved"); auto b = makeSnap("SN-04", "rejected");
    auto r = DecisionReplayChecker::replay(a, b);
    C(!r.fullyReproducible && r.divergenceReason == "decision_changed", "decision");
    P();
}
void t5() {
    T(validate_valid_snapshot);
    auto s = makeSnap("SN-05");
    std::string err;
    C(DecisionReplayChecker::validate(s, &err), "valid");
    P();
}
void t6() {
    T(validate_missing_snapshot_id);
    DecisionSnapshot s; s.issueRef = "ISS-01"; s.decision = "approved";
    std::string err;
    C(!DecisionReplayChecker::validate(s, &err) && err == "snapshot_id_missing", "err");
    P();
}
void t7() {
    T(validate_missing_decision);
    DecisionSnapshot s; s.snapshotId = "SN-07"; s.issueRef = "ISS-01";
    std::string err;
    C(!DecisionReplayChecker::validate(s, &err) && err == "decision_missing", "err");
    P();
}
void t8() {
    T(batch_replay);
    auto s1 = makeSnap("SN-08a"); auto s2 = makeSnap("SN-08b");
    auto c1 = s1; auto c2 = s2; c2.decision = "rejected";
    auto results = DecisionReplayChecker::batchReplay({s1, s2}, {c1, c2});
    C(results.size() == 2, "size");
    C(results[0].fullyReproducible && !results[1].fullyReproducible, "results");
    P();
}
void t9() {
    T(count_reproducible);
    auto s = makeSnap("SN-09");
    std::vector<DecisionSnapshot> archived = {s, s};
    auto other = s; other.decision = "rejected";
    std::vector<DecisionSnapshot> current = {s, other};
    auto results = DecisionReplayChecker::batchReplay(archived, current);
    C(DecisionReplayChecker::countReproducible(results) == 1, "count");
    P();
}
void t10() {
    T(to_json);
    auto s = makeSnap("SN-10");
    auto r = DecisionReplayChecker::replay(s, s);
    auto j = DecisionReplayChecker::toJson(r);
    C(j.value("fully_reproducible", false), "repro");
    C(j.value("snapshot_id", "") == "SN-10", "id");
    P();
}

int main() {
    std::cout << "Step 824: Decision replay and reproducibility checker\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f ? 1 : 0;
}
