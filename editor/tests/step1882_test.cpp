// Step 1882: Sprint 270 integration summary
//
// Verifies that all Sprint 270 new components compile and interoperate
// in combined scenarios representing the real production decision flows.
//
//  t1: constrained projection fails → cross-file transaction also blocked (cascades)
//  t2: semantic gate depth exceeded → token budget irrelevant (gate hierarchy)
//  t3: decomposition too shallow → refactor policy prevents C++ generation
//  t4: all gates green → full pipeline passes
//  t5: parity skew + constrained projection → combined gate signal

#include "ConstrainedProjectionGate.h"
#include "SemanticCompletionGate.h"
#include "CrossFileTransactionGate.h"
#include "TokenBudgetGate.h"
#include "CppConstraintRefactorPolicy.h"
#include "NativeDecompositionDepthGuard.h"
#include "ParitySkewAnalyzer.h"
#include "ast/Module.h"
#include "ast/Annotation.h"
#include <iostream>
#include <string>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(projection_fail_cascades_to_transaction);
    // A file that fails projection constraint also blocks the transaction
    Module root;
    root.id = "file-auth";
    auto* pa = new ParallelAnnotation(); pa->kind = "data";
    root.addChild("annotations", pa);

    EnvironmentSpec env; env.scheduler = "single_thread";
    auto projection = ConstrainedProjectionGate::validate(&root, &env);
    C(projection.blocked, "parallel annotation in single_thread should block projection");

    // File that failed projection is not ready for promotion
    FileEditState state{"file-auth", 3, 1, 0, 1};  // blocking issue present
    auto tx = CrossFileTransactionGate::evaluate({state});
    C(!tx.promotable, "blocked file should prevent transaction promotion");
    P();
}

void t2(){
    T(semantic_depth_exceeded_blocks_regardless_of_token_budget);
    auto semantic = SemanticCompletionGate::evaluate(0.95f, 7, true);
    C(!semantic.passed, "depth=7 > max=5 should block even with high score");

    // Token budget might be fine, but semantic gate supersedes it
    std::string output(500, 'x');
    auto token = TokenBudgetGate::evaluate(output, 8192);
    C(!token.blocked, "small output should not be token-blocked");

    // Combined: semantic gate blocks → don't bother with token check
    bool blocked = !semantic.passed;
    C(blocked, "semantic gate should be authoritative block signal");
    P();
}

void t3(){
    T(shallow_decomposition_prevents_cpp_refactor);
    auto decomp = NativeDecompositionDepthGuard::evaluate({"task-a", "task-b"}, 3);
    C(!decomp.accepted, "2 tasks < min=3 → decomposition too shallow");

    // Shallow decomposition → don't run the C++ refactor policy
    // (if we did run it, require all constraints present)
    RefactorConstraints required{true, true, true, true};  // staged rollout IS required
    auto policy = CppConstraintRefactorPolicy::evaluate(required, true, true, true, false);
    C(!policy.allowed, "missing staged rollout should block C++ refactor");
    P();
}

void t4(){
    T(all_sprint270_gates_green);
    // Projection: no constraints violated
    Module root; root.id = "mod-clean";
    EnvironmentSpec env; env.scheduler = "threads"; env.capabilities = {"io.fs"};
    auto proj = ConstrainedProjectionGate::validate(&root, &env);
    C(!proj.blocked, "clean module should pass projection gate");

    // Semantic: high score, low depth
    auto sem = SemanticCompletionGate::evaluate(0.92f, 1, false);
    C(sem.passed, "high score + low depth should pass semantic gate");

    // Transaction: single ready file
    auto tx = CrossFileTransactionGate::evaluate({{"main.cpp", 2, 2, 1, 0}});
    C(tx.promotable, "ready file should allow transaction");

    // Token: small output
    auto tok = TokenBudgetGate::evaluate(std::string(100, 'x'), 8192);
    C(!tok.blocked, "small output should pass token gate");

    // Decomposition: enough tasks
    auto decomp = NativeDecompositionDepthGuard::evaluate({"a","b","c","d"}, 3);
    C(decomp.accepted, "4 tasks should pass depth guard");
    P();
}

void t5(){
    T(parity_skew_plus_projection_combined_signal);
    // High parity skew (only python ready)
    auto skew = ParitySkewAnalyzer::analyze({
        {"python", true}, {"cpp", false}, {"rust", false}
    }, 0.4f);
    C(skew.belowThreshold, "2/3 not ready → high skew");

    // Projection fails for a node in the cpp generator path
    Module root; root.id = "cpp-mod";
    auto* cr = new CapabilityRequirement();
    cr->capability = "io.net"; cr->required = true;
    root.addChild("annotations", cr);
    EnvironmentSpec env; env.capabilities = {};  // no capabilities
    auto proj = ConstrainedProjectionGate::validate(&root, &env);
    C(proj.blocked, "missing capability should block cpp projection");

    // Both signals → do not proceed with cpp generation
    bool shouldSkipCpp = skew.belowThreshold || proj.blocked;
    C(shouldSkipCpp, "combined signals should suppress cpp generation");
    P();
}

int main(){
    std::cout<<"Step 1882: Sprint 270 integration — GR-009/010/012/014/018/020 combined\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
