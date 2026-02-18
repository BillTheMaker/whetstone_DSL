// Step 647: Self-modification safety guard (12 tests)

#include "SelfModificationSafetyGuard.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(live_path_detected_for_editor_src);C(SelfModificationSafetyGuard::isLiveBinaryPath("editor/src/main.cpp",{"editor/src"}),"should detect live path");P();}
void t2(){T(non_live_path_not_detected);C(!SelfModificationSafetyGuard::isLiveBinaryPath("docs/readme.md",{"editor/src"}),"should not detect");P();}
void t3(){T(empty_live_roots_allows_non_detection);C(!SelfModificationSafetyGuard::isLiveBinaryPath("editor/src/main.cpp",{}),"empty roots should not match");P();}
void t4(){T(allow_mutation_true_for_non_live_file);std::string r;C(SelfModificationSafetyGuard::allowMutation("docs/readme.md",{"editor/src"},&r),"should allow");P();}
void t5(){T(allow_mutation_false_for_live_file);std::string r;C(!SelfModificationSafetyGuard::allowMutation("editor/src/main.cpp",{"editor/src"},&r),"should block");P();}
void t6(){T(block_reason_contains_required_message);std::string r;SelfModificationSafetyGuard::allowMutation("editor/src/main.cpp",{"editor/src"},&r);C(r.find("Cannot mutate live binary")!=std::string::npos,"reason missing");P();}
void t7(){T(allow_mutation_clears_reason_on_success);std::string r="x";SelfModificationSafetyGuard::allowMutation("docs/x",{"editor/src"},&r);C(r.empty(),"reason should clear");P();}
void t8(){T(null_reason_pointer_rejected);C(!SelfModificationSafetyGuard::allowMutation("docs/x",{"editor/src"},nullptr),"null reason should fail");P();}
void t9(){T(multi_root_live_match_detected);C(SelfModificationSafetyGuard::isLiveBinaryPath("editor/tests/a.cpp",{"editor/src","editor/tests"}),"multi-root should match");P();}
void t10(){T(prefix_match_respects_root_start);C(!SelfModificationSafetyGuard::isLiveBinaryPath("tmp/editor/src/main.cpp",{"editor/src"}),"must match prefix");P();}
void t11(){T(empty_path_is_allowed);std::string r;C(SelfModificationSafetyGuard::allowMutation("",{"editor/src"},&r),"empty path should allow");P();}
void t12(){T(root_empty_is_ignored);C(!SelfModificationSafetyGuard::isLiveBinaryPath("editor/src/main.cpp",{""}),"empty root ignored");P();}

int main(){std::cout<<"Step 647: Self-modification safety guard\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
