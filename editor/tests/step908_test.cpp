// Step 908: Sprint 67 integration summary (6 tests)
#include "Sprint67IntegrationSummary.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(sprint_number);
    C(Sprint67IntegrationSummary::sprintNumber==67,"num");P();}
void t2(){T(steps_completed);
    C(Sprint67IntegrationSummary::stepsCompleted==10,"steps");P();}
void t3(){T(theme_set);
    std::string t=Sprint67IntegrationSummary::theme;
    C(!t.empty(),"theme");P();}
void t4(){T(verify_true);
    C(Sprint67IntegrationSummary::verify(),"verify");P();}
void t5(){T(to_json_sprint_field);
    auto j=Sprint67IntegrationSummary::toJson();
    C(j.contains("sprint")&&j["sprint"]==67,"json");P();}
void t6(){T(to_json_tools_added);
    auto j=Sprint67IntegrationSummary::toJson();
    C(j.contains("tools_added"),"tools");P();}

int main(){
    std::cout<<"Step 908: Sprint 67 integration summary\n";
    t1();t2();t3();t4();t5();t6();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
