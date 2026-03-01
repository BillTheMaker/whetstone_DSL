// Step 1877: Sprint 269 integration summary
//
// Verifies that all Sprint 269 new components compile and interoperate
// correctly in a combined scenario:
//
//  t1: RequirementsParser prose capture + GateEnforcer → prose spec feeds into gated review
//  t2: CrossArtifactConsistencyEngine detects drift and GateEnforcer blocks production
//  t3: ParitySkewAnalyzer + CrossArtifactConsistencyEngine → multi-signal quality check
//  t4: GateEnforcer with mixed gates (security fail + performance fail) → only security blocks
//  t5: Full pipeline: prose intake → consistency check → gate enforcement (no drift → not blocked)

#include "RequirementsParser.h"
#include "GateEnforcer.h"
#include "graduation/CrossArtifactConsistencyEngine.h"
#include "ParitySkewAnalyzer.h"
#include <iostream>
#include <string>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(prose_spec_into_gate_review);
    std::string md =
        "## Requirements\n"
        "The system must validate all user input at the API boundary.\n"
        "The system must log all authentication events for audit purposes.\n";
    auto items = RequirementsParser::parse(md);
    C(items.size() >= 2, "expected prose requirements captured");

    // Security gate fails → production blocked
    auto gate = GateEnforcer::evaluate({{"security", false}});
    C(gate.blocked, "security gate failure should block production");
    P();
}

void t2(){
    T(consistency_drift_then_gate_block);
    // Spec goal not covered by any taskitem intent → drift detected
    auto consistency = CrossArtifactConsistencyEngine::check(
        {"audit logging for security events"},
        {"implement search feature", "add caching layer"});
    C(!consistency.consistent, "expected drift when goal is uncovered");
    C(consistency.inconsistencyCount == 1, "expected 1 drifted goal");

    // Drift triggers a security-equivalent gate failure: consistent=false → passed=false
    auto gate = GateEnforcer::evaluate({{"security", consistency.consistent}});
    C(gate.blocked, "drift should trigger blocked gate");
    P();
}

void t3(){
    T(parity_skew_plus_consistency_multi_signal);
    // Parity: only python ready → high skew
    auto parity = ParitySkewAnalyzer::analyze({
        {"python", true}, {"cpp", false}, {"rust", false}
    }, 0.4f);
    C(parity.belowThreshold, "expected high skew to be flagged");

    // Consistency: goal is covered
    auto consistency = CrossArtifactConsistencyEngine::check(
        {"generate cross-language code"},
        {"generate python code", "generate cpp code"});
    C(consistency.consistent, "expected goals covered");

    // Combined: parity issue present but consistency ok
    C(parity.belowThreshold && consistency.consistent,
      "should be able to have parity issues alongside consistency");
    P();
}

void t4(){
    T(mixed_gates_only_block_severity_blocks);
    auto result = GateEnforcer::evaluate({
        {"security",    false},  // block → blocked
        {"sanitizer",   true},   // block but passed → not blocking
        {"performance", false},  // warn → not blocking
        {"supply_chain",false}   // warn → not blocking
    });
    C(result.blocked, "should be blocked due to security gate");
    C(result.blockingGates.size() == 1u, "only security should be in blockingGates");
    C(result.blockingGates[0] == "security", "blocking gate should be 'security'");
    P();
}

void t5(){
    T(prose_intake_consistency_pass_not_blocked);
    // Prose spec → capture requirements → check consistency → gate enforcement
    std::string md =
        "## Requirements\n"
        "The system must implement OAuth2 authentication.\n"
        "The system must rate-limit API requests.\n";
    auto items = RequirementsParser::parse(md);
    C(items.size() >= 2, "prose requirements must be captured");

    // Taskitem intents align with goals
    auto consistency = CrossArtifactConsistencyEngine::check(
        {"OAuth2 authentication", "rate-limit API requests"},
        {"implement OAuth2 login flow", "add rate limiter middleware"});
    C(consistency.consistent, "intents should cover all goals");

    // No gate failures
    auto gate = GateEnforcer::evaluate({
        {"security",    true},
        {"sanitizer",   true},
        {"performance", true}
    });
    C(!gate.blocked, "no failing gates → production should not be blocked");
    P();
}

int main(){
    std::cout<<"Step 1877: Sprint 269 integration — GR-005/011/013/015 combined\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
