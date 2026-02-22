// Step 837: Sprint60IntegrationSummary tests (8 tests)
#include "Sprint60IntegrationSummary.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(sprintNumber);C(Sprint60IntegrationSummary::sprintNumber==60,"60");P();}
void t2(){T(stepsCompleted);C(Sprint60IntegrationSummary::stepsCompleted==9,"9");P();}
void t3(){T(theme_not_empty);C(std::string(Sprint60IntegrationSummary::theme).size()>0,"theme");P();}
void t4(){T(verify_true);C(Sprint60IntegrationSummary::verify(),"verify");P();}
void t5(){T(json_has_sprint_key);auto j=Sprint60IntegrationSummary::toJson();C(j.contains("sprint"),"sprint");P();}
void t6(){T(json_sprint_60);auto j=Sprint60IntegrationSummary::toJson();C(j["sprint"]==60,"sprint=60");P();}
void t7(){T(json_steps_9);auto j=Sprint60IntegrationSummary::toJson();C(j["steps"]==9,"steps=9");P();}
void t8(){T(json_status_complete);auto j=Sprint60IntegrationSummary::toJson();C(j["status"]=="complete","complete");P();}

int main(){
    std::cout<<"Step 837: Sprint60IntegrationSummary\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
