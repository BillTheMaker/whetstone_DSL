// Step 1874 TDD: GR-011 — GateEnforcer enforces block-severity gates as hard fails
//
// GateSeverityPolicy classifies "security" and "sanitizer" as "block" severity.
// Previously nothing enforced this — gates were advisory. GateEnforcer adds
// the enforcement: a batch of gate results → blocked=true if any block-severity gate fails.
//
//  t1: failed security gate → blocked=true, gate in blockingGates
//  t2: failed sanitizer gate → blocked=true
//  t3: failed performance gate → NOT blocked (warn severity)
//  t4: failed unknown gate → NOT blocked (info severity)
//  t5: all gates pass → blocked=false, blockingGates empty

#include "GateEnforcer.h"
#include <iostream>
#include <string>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(failed_security_gate_blocks);
    auto result = GateEnforcer::evaluate({{"security", false}});
    C(result.blocked, "failed security gate should block production");
    C(!result.blockingGates.empty(), "blockingGates should be non-empty");
    C(result.blockingGates[0] == "security", "expected 'security' in blockingGates");
    P();
}

void t2(){
    T(failed_sanitizer_gate_blocks);
    auto result = GateEnforcer::evaluate({{"sanitizer", false}});
    C(result.blocked, "failed sanitizer gate should block production");
    P();
}

void t3(){
    T(failed_performance_gate_does_not_block);
    auto result = GateEnforcer::evaluate({{"performance", false}});
    C(!result.blocked, "failed performance gate is warn-severity, should not block");
    C(result.blockingGates.empty(), "blockingGates should be empty for warn gates");
    P();
}

void t4(){
    T(failed_unknown_gate_does_not_block);
    auto result = GateEnforcer::evaluate({{"unknown_gate_type", false}});
    C(!result.blocked, "unknown gate is info-severity, should not block");
    P();
}

void t5(){
    T(all_gates_pass_no_block);
    auto result = GateEnforcer::evaluate({
        {"security", true},
        {"sanitizer", true},
        {"performance", true}
    });
    C(!result.blocked, "all passing gates should not block");
    C(result.blockingGates.empty(), "blockingGates should be empty when all pass");
    P();
}

int main(){
    std::cout<<"Step 1874: GR-011 GateEnforcer block-severity enforcement\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
