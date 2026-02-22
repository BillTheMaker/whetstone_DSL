// Step 823: Policy pack system (safe-first, interop-first, custom) (10 tests)
#include "governance/PolicyPackSystem.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

void t1() {
    T(build_safe_first);
    auto pack = PolicyPackSystem::buildSafeFirst();
    C(pack.packId == "safe-first", "id");
    C(!pack.allowWaivers, "no_waivers");
    C(!pack.rules.empty(), "rules");
    P();
}
void t2() {
    T(build_interop_first);
    auto pack = PolicyPackSystem::buildInteropFirst();
    C(pack.packId == "interop-first", "id");
    C(pack.allowWaivers, "waivers");
    P();
}
void t3() {
    T(build_custom);
    std::vector<PolicyRule> rules = {{"R1", "custom rule", true}};
    auto pack = PolicyPackSystem::buildCustom("my-pack", rules);
    C(pack.type == PolicyPackType::Custom, "type");
    C(pack.rules.size() == 1, "rules");
    P();
}
void t4() {
    T(validate_valid_pack);
    auto pack = PolicyPackSystem::buildSafeFirst();
    std::string err;
    C(PolicyPackSystem::validate(pack, &err), "valid");
    P();
}
void t5() {
    T(validate_empty_rules);
    PolicyPack pack; pack.packId = "bad";
    std::string err;
    C(!PolicyPackSystem::validate(pack, &err) && err == "no_rules", "err");
    P();
}
void t6() {
    T(validate_missing_id);
    PolicyPack pack; pack.rules = {{"R1", "desc", true}};
    std::string err;
    C(!PolicyPackSystem::validate(pack, &err) && err == "pack_id_missing", "err");
    P();
}
void t7() {
    T(evaluate_blocking_rule);
    auto pack = PolicyPackSystem::buildSafeFirst();
    std::string err;
    bool blocking = PolicyPackSystem::evaluate(pack, "SF-01", &err);
    C(blocking, "blocking");
    P();
}
void t8() {
    T(evaluate_nonexistent_rule);
    auto pack = PolicyPackSystem::buildSafeFirst();
    std::string err;
    PolicyPackSystem::evaluate(pack, "NOPE", &err);
    C(err == "rule_not_found", "err");
    P();
}
void t9() {
    T(type_str_values);
    C(PolicyPackSystem::typeStr(PolicyPackType::SafeFirst) == "safe-first", "sf");
    C(PolicyPackSystem::typeStr(PolicyPackType::InteropFirst) == "interop-first", "if");
    C(PolicyPackSystem::typeStr(PolicyPackType::Custom) == "custom", "custom");
    P();
}
void t10() {
    T(to_json);
    auto pack = PolicyPackSystem::buildSafeFirst();
    auto j = PolicyPackSystem::toJson(pack);
    C(j.value("pack_id", "") == "safe-first", "id");
    C(j.value("type", "") == "safe-first", "type");
    C(j.contains("rules"), "rules");
    P();
}

int main() {
    std::cout << "Step 823: Policy pack system\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f ? 1 : 0;
}
