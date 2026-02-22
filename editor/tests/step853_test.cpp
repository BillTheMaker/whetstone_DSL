// Step 853: PrioritizationScorer tests (10 tests)
#include "graduation/PrioritizationScorer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(critical_score);
    PrioritizationInput in{"T001",10,10,3};
    auto s=PrioritizationScorer::score(in);
    C(s.priority=="critical","critical");P();}

void t2(){T(high_score);
    PrioritizationInput in{"T001",5,4,2};
    auto s=PrioritizationScorer::score(in);
    C(s.priority=="high","high");P();}

void t3(){T(medium_score);
    PrioritizationInput in{"T001",2,3,2};
    auto s=PrioritizationScorer::score(in);
    C(s.priority=="medium","medium");P();}

void t4(){T(low_score);
    PrioritizationInput in{"T001",1,1,1};
    auto s=PrioritizationScorer::score(in);
    C(s.priority=="low","low");P();}

void t5(){T(score_formula);
    PrioritizationInput in{"T001",5,4,3};
    auto s=PrioritizationScorer::score(in);
    C(s.score==60,"score=5*4*3=60");P();}

void t6(){T(code_preserved);
    PrioritizationInput in{"T004",5,5,2};
    auto s=PrioritizationScorer::score(in);
    C(s.code=="T004","code");P();}

void t7(){T(rank_order);
    std::vector<PrioritizationScore> scores={
        PrioritizationScorer::score({"T001",1,1,1}),
        PrioritizationScorer::score({"T002",10,10,3})};
    auto ranked=PrioritizationScorer::rank(scores);
    C(ranked[0].code=="T002","T002 first");P();}

void t8(){T(zero_score_low);
    PrioritizationInput in{"T001",0,0,0};
    auto s=PrioritizationScorer::score(in);
    C(s.priority=="low","low");P();}

void t9(){T(toJson);
    PrioritizationInput in{"T001",5,5,2};
    auto s=PrioritizationScorer::score(in);
    auto j=PrioritizationScorer::toJson(s);
    C(j.contains("score")&&j.contains("priority"),"keys");P();}

void t10(){T(score_boundary_100_critical);
    PrioritizationInput in{"T001",10,10,1};
    auto s=PrioritizationScorer::score(in);
    C(s.priority=="critical","critical at 100");P();}

int main(){
    std::cout<<"Step 853: PrioritizationScorer\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
