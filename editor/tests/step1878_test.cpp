// Step 1878 TDD: GR-012 — ConstrainedProjectionGate blocks on environment violations
//
// ProjectionGenerator.generate() took only const ASTNode* — no EnvironmentSpec access.
// ConstrainedProjectionGate runs validateCapabilities + validateEnvAnnotations before
// generation and returns blocked=true when any error-severity diagnostic is emitted.
//
//  t1: node requires capability not in env → blocked=true
//  t2: node requires capability present in env → not blocked
//  t3: ParallelAnnotation in single_thread env → blocked (E0502)
//  t4: null env → not blocked (no constraints)
//  t5: warning-only diagnostics (AtomicAnnotation in single_thread) → not blocked

#include "ConstrainedProjectionGate.h"
#include "ast/Module.h"
#include "ast/Annotation.h"
#include <iostream>
#include <string>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static EnvironmentSpec makeEnv(const std::string& scheduler = "threads",
                                const std::vector<std::string>& caps = {}) {
    EnvironmentSpec env;
    env.envId = "test_env";
    env.scheduler = scheduler;
    env.capabilities = caps;
    return env;
}

void t1(){
    T(missing_capability_blocks);
    Module root;
    root.id = "mod-t1";

    // Add a CapabilityRequirement for "io.net" which is NOT in the env
    auto* cr = new CapabilityRequirement();
    cr->capability = "io.net";
    cr->required = true;
    root.addChild("annotations", cr);

    EnvironmentSpec env = makeEnv("threads", {"io.fs"});  // has io.fs, not io.net
    auto result = ConstrainedProjectionGate::validate(&root, &env);
    C(result.blocked, "missing required capability should block projection");
    C(!result.diagnostics.empty(), "expected diagnostics for missing capability");
    P();
}

void t2(){
    T(satisfied_capability_not_blocked);
    Module root;
    root.id = "mod-t2";

    auto* cr = new CapabilityRequirement();
    cr->capability = "io.fs";
    cr->required = true;
    root.addChild("annotations", cr);

    EnvironmentSpec env = makeEnv("threads", {"io.fs", "io.net"});
    auto result = ConstrainedProjectionGate::validate(&root, &env);
    C(!result.blocked, "satisfied capability should not block projection");
    P();
}

void t3(){
    T(parallel_annotation_in_single_thread_blocks);
    Module root;
    root.id = "mod-t3";

    auto* pa = new ParallelAnnotation();
    pa->kind = "data";
    root.addChild("annotations", pa);

    EnvironmentSpec env = makeEnv("single_thread", {});
    auto result = ConstrainedProjectionGate::validate(&root, &env);
    C(result.blocked, "ParallelAnnotation in single_thread env should block (E0502)");
    bool foundE0502 = false;
    for (const auto& d : result.diagnostics) {
        if (d.message.find("E0502") != std::string::npos) foundE0502 = true;
    }
    C(foundE0502, "expected E0502 diagnostic");
    P();
}

void t4(){
    T(null_env_not_blocked);
    Module root;
    root.id = "mod-t4";
    // Even with a capability requirement, null env means no constraints
    auto* cr = new CapabilityRequirement();
    cr->capability = "io.net";
    cr->required = true;
    root.addChild("annotations", cr);

    auto result = ConstrainedProjectionGate::validate(&root, nullptr);
    C(!result.blocked, "null env should never block");
    C(result.diagnostics.empty(), "null env should produce no diagnostics");
    P();
}

void t5(){
    T(warning_only_diagnostics_not_blocked);
    Module root;
    root.id = "mod-t5";

    // AtomicAnnotation in single_thread produces a warning, not an error
    auto* aa = new AtomicAnnotation();
    root.addChild("annotations", aa);

    EnvironmentSpec env = makeEnv("single_thread", {});
    auto result = ConstrainedProjectionGate::validate(&root, &env);
    C(!result.blocked, "warning-only diagnostics should not block projection");
    bool foundWarning = false;
    for (const auto& d : result.diagnostics) {
        if (d.severity == "warning") foundWarning = true;
    }
    C(foundWarning, "expected at least one warning diagnostic (E0504)");
    P();
}

int main(){
    std::cout<<"Step 1878: GR-012 ConstrainedProjectionGate environment constraint enforcement\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
