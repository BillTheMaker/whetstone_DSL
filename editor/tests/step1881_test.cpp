// Step 1881 TDD: GR-014 + GR-018 + GR-020 — Token budget, refactor policy, decomposition depth
//
//  GR-014 (TokenBudgetGate): blocks when token estimate exceeds budget or yield too low
//  GR-018 (CppConstraintRefactorPolicy): enforces rollback/security/SLO/rollout constraints
//  GR-020 (NativeDecompositionDepthGuard): rejects decompositions with fewer than min tasks
//
//  t1 (GR-014): output within budget → not blocked
//  t2 (GR-014): output exceeds budget → blocked
//  t3 (GR-018): all required constraints satisfied → allowed
//  t4 (GR-018): missing rollback plan + missing security review → violations listed
//  t5 (GR-020): 4 tasks with min=3 → accepted; 2 tasks with min=3 → rejected

#include "TokenBudgetGate.h"
#include "CppConstraintRefactorPolicy.h"
#include "NativeDecompositionDepthGuard.h"
#include <iostream>
#include <string>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(token_within_budget_not_blocked);
    // Small output, large context window → well within budget
    std::string output(200, 'x');  // ~50 tokens estimated
    auto result = TokenBudgetGate::evaluate(output, 8192);
    C(!result.blocked, "small output within budget should not be blocked");
    C(result.estimatedTokens > 0, "estimated tokens should be positive");
    C(result.budgetTokens > result.estimatedTokens, "budget should exceed estimated tokens");
    P();
}

void t2(){
    T(token_exceeds_budget_blocked);
    // Output larger than the entire context window budget
    std::string output(100000, 'x');  // ~25000 tokens
    auto result = TokenBudgetGate::evaluate(output, 1024);  // budget ~614 tokens
    C(result.blocked, "output exceeding budget should be blocked");
    C(result.estimatedTokens > result.budgetTokens,
      "estimated should exceed budget, est=" + std::to_string(result.estimatedTokens) +
      " budget=" + std::to_string(result.budgetTokens));
    P();
}

void t3(){
    T(cpp_refactor_all_constraints_satisfied);
    RefactorConstraints required{true, true, true, true};
    auto result = CppConstraintRefactorPolicy::evaluate(required,
        /*rollback*/true, /*security*/true, /*slo*/true, /*staged*/true);
    C(result.allowed, "all constraints satisfied → refactor should be allowed");
    C(result.violations.empty(), "expected no violations");
    P();
}

void t4(){
    T(cpp_refactor_missing_constraints_blocked);
    RefactorConstraints required{true, true, false, false};
    auto result = CppConstraintRefactorPolicy::evaluate(required,
        /*rollback*/false, /*security*/false, /*slo*/true, /*staged*/true);
    C(!result.allowed, "missing required constraints should block refactor");
    C(result.violations.size() == 2u,
      "expected 2 violations, got " + std::to_string(result.violations.size()));
    bool hasRollback = false, hasSecurity = false;
    for (const auto& v : result.violations) {
        if (v == "missing_rollback_plan") hasRollback = true;
        if (v == "missing_security_review") hasSecurity = true;
    }
    C(hasRollback, "expected missing_rollback_plan violation");
    C(hasSecurity, "expected missing_security_review violation");
    P();
}

void t5(){
    T(native_decomposition_depth_guard);
    // 4 tasks with min=3 → accepted
    auto ok = NativeDecompositionDepthGuard::evaluate(
        {"task-auth", "task-session", "task-db", "task-audit"}, 3);
    C(ok.accepted, "4 tasks with min=3 should be accepted");
    C(ok.reason == "depth_sufficient", "expected depth_sufficient, got " + ok.reason);

    // 2 tasks with min=3 → rejected
    auto fail = NativeDecompositionDepthGuard::evaluate({"task-a", "task-b"}, 3);
    C(!fail.accepted, "2 tasks with min=3 should be rejected (too shallow)");
    C(fail.reason == "decomposition_too_shallow",
      "expected decomposition_too_shallow, got " + fail.reason);
    P();
}

int main(){
    std::cout<<"Step 1881: GR-014/GR-018/GR-020 token budget + refactor policy + decomp guard\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
