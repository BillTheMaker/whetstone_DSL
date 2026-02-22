// Step 821: Waiver policy packet (temporary/permanent/scoped) (10 tests)
#include "governance/WaiverPolicyPacket.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

void t1() {
    T(make_temporary_waiver);
    auto w = WaiverPolicyPacketModel::make("W1", "ISS-01", WaiverScope::Temporary, "rationale", "alice");
    w.expiresAt = "2026-12-31";
    std::string err;
    C(WaiverPolicyPacketModel::validate(w, &err), "valid");
    C(w.active, "active");
    P();
}
void t2() {
    T(make_permanent_waiver);
    auto w = WaiverPolicyPacketModel::make("W2", "ISS-02", WaiverScope::Permanent, "rationale", "bob");
    std::string err;
    C(WaiverPolicyPacketModel::validate(w, &err), "valid");
    P();
}
void t3() {
    T(scoped_waiver_needs_path);
    auto w = WaiverPolicyPacketModel::make("W3", "ISS-03", WaiverScope::Scoped, "rationale", "carol");
    std::string err;
    C(!WaiverPolicyPacketModel::validate(w, &err) && err == "scope_path_missing", "err");
    P();
}
void t4() {
    T(scoped_waiver_with_path);
    auto w = WaiverPolicyPacketModel::make("W4", "ISS-04", WaiverScope::Scoped, "rationale", "carol");
    w.scopePath = "/module/foo";
    std::string err;
    C(WaiverPolicyPacketModel::validate(w, &err), "valid");
    P();
}
void t5() {
    T(temporary_needs_expires_at);
    auto w = WaiverPolicyPacketModel::make("W5", "ISS-05", WaiverScope::Temporary, "rationale", "alice");
    std::string err;
    C(!WaiverPolicyPacketModel::validate(w, &err) && err == "expires_at_missing", "err");
    P();
}
void t6() {
    T(revoke_active_waiver);
    auto w = WaiverPolicyPacketModel::make("W6", "ISS-06", WaiverScope::Permanent, "rationale", "alice");
    std::string err;
    C(WaiverPolicyPacketModel::revoke(w, &err), "revoke");
    C(!w.active, "inactive");
    P();
}
void t7() {
    T(revoke_already_inactive);
    auto w = WaiverPolicyPacketModel::make("W7", "ISS-07", WaiverScope::Permanent, "rationale", "alice");
    w.active = false;
    std::string err;
    C(!WaiverPolicyPacketModel::revoke(w, &err) && err == "already_inactive", "err");
    P();
}
void t8() {
    T(missing_waiver_id);
    WaiverPolicyPacket w; w.issueRef = "ISS-08"; w.rationale = "x"; w.grantedBy = "alice";
    std::string err;
    C(!WaiverPolicyPacketModel::validate(w, &err) && err == "waiver_id_missing", "err");
    P();
}
void t9() {
    T(scope_str_values);
    C(WaiverPolicyPacketModel::scopeStr(WaiverScope::Temporary) == "temporary", "temp");
    C(WaiverPolicyPacketModel::scopeStr(WaiverScope::Permanent) == "permanent", "perm");
    C(WaiverPolicyPacketModel::scopeStr(WaiverScope::Scoped) == "scoped", "scoped");
    P();
}
void t10() {
    T(to_json);
    auto w = WaiverPolicyPacketModel::make("W10", "ISS-10", WaiverScope::Permanent, "rationale", "alice");
    auto j = WaiverPolicyPacketModel::toJson(w);
    C(j.value("waiver_id", "") == "W10", "id");
    C(j.value("scope", "") == "permanent", "scope");
    C(j.value("active", false), "active");
    P();
}

int main() {
    std::cout << "Step 821: Waiver policy packet\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f ? 1 : 0;
}
