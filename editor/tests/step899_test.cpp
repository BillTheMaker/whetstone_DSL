// Step 899: Migration path candidate schema (8 tests)
#include "graduation/MigrationPathCandidate.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(pair_id_set);
    auto c=MigrationPathCandidateFactory::make("py->cpp","cpython3","gcc",{"a","b"},0.8f,"ok");
    C(c.pairId=="py->cpp","id");P();}
void t2(){T(source_runtime_set);
    auto c=MigrationPathCandidateFactory::make("py->cpp","cpython3","gcc",{"a"},0.8f,"ok");
    C(c.sourceRuntime=="cpython3","src");P();}
void t3(){T(target_runtime_set);
    auto c=MigrationPathCandidateFactory::make("py->cpp","cpython3","gcc",{"a"},0.8f,"ok");
    C(c.targetRuntime=="gcc","tgt");P();}
void t4(){T(feasibility_set);
    auto c=MigrationPathCandidateFactory::make("py->cpp","cpython3","gcc",{"a"},0.75f,"ok");
    C(c.feasibility==0.75f,"feas");P();}
void t5(){T(steps_set);
    auto c=MigrationPathCandidateFactory::make("py->cpp","cpython3","gcc",{"s1","s2"},0.8f,"ok");
    C(c.steps.size()==2,"steps");P();}
void t6(){T(rationale_set);
    auto c=MigrationPathCandidateFactory::make("py->cpp","cpython3","gcc",{"a"},0.8f,"reason");
    C(c.rationale=="reason","rat");P();}
void t7(){T(to_json_has_pair_id);
    auto c=MigrationPathCandidateFactory::make("py->cpp","cpython3","gcc",{"a"},0.8f,"ok");
    auto j=MigrationPathCandidateFactory::toJson(c);
    C(j.contains("pair_id"),"json");P();}
void t8(){T(to_json_has_feasibility);
    auto c=MigrationPathCandidateFactory::make("py->cpp","cpython3","gcc",{"a"},0.8f,"ok");
    auto j=MigrationPathCandidateFactory::toJson(c);
    C(j.contains("feasibility"),"json");P();}

int main(){
    std::cout<<"Step 899: Migration path candidate schema\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
