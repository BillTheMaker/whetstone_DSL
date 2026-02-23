// Step 902: Migration step sequencer (8 tests)
#include "graduation/MigrationStepSequencer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

// Step IDs are prefixed with pairId: "p1:analyze_source" etc.
void t1(){T(at_least_three_steps);
    auto s=MigrationStepSequencer::sequence("p1","jvm11","cpython3");
    C(s.size()>=3,"three");P();}
void t2(){T(first_step_has_analyze_source);
    auto s=MigrationStepSequencer::sequence("p1","jvm11","cpython3");
    C(s[0].stepId.find("analyze_source")!=std::string::npos,"first");P();}
void t3(){T(second_step_has_adapt_semantics);
    auto s=MigrationStepSequencer::sequence("p1","jvm11","cpython3");
    C(s[1].stepId.find("adapt_semantics")!=std::string::npos,"second");P();}
void t4(){T(third_step_has_verify_target);
    auto s=MigrationStepSequencer::sequence("p1","jvm11","cpython3");
    C(s[2].stepId.find("verify_target")!=std::string::npos,"third");P();}
void t5(){T(steps_have_description);
    auto s=MigrationStepSequencer::sequence("p1","jvm11","cpython3");
    C(!s[0].description.empty(),"desc");P();}
void t6(){T(first_step_no_prerequisite);
    auto s=MigrationStepSequencer::sequence("p1","jvm11","cpython3");
    C(s[0].prerequisite.empty(),"no prereq");P();}
void t7(){T(second_step_required);
    auto s=MigrationStepSequencer::sequence("p1","jvm11","cpython3");
    C(s[1].required,"required");P();}
void t8(){T(to_json_has_step_id);
    auto s=MigrationStepSequencer::sequence("p1","jvm11","cpython3");
    auto j=MigrationStepSequencer::toJson(s[0]);
    C(j.contains("step_id"),"json");P();}

int main(){
    std::cout<<"Step 902: Migration step sequencer\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
