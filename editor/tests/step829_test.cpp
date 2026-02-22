// Step 829: PairMatrixRunner tests (12 tests)
#include "graduation/PairMatrixRunner.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(run_single_pair);
    LanguagePair lp{"python","cpp","stable"};
    auto r=PairMatrixRunner::runPair(lp);
    C(r.passed,"should pass");P();}

void t2(){T(run_all_pairs);
    std::vector<LanguagePair> pairs={{"python","cpp","stable"},{"rust","go","beta"}};
    auto s=PairMatrixRunner::runAll(pairs);
    C(s.totalPairs==2,"totalPairs");P();}

void t3(){T(summary_total_count);
    std::vector<LanguagePair> pairs={{"a","b","stable"},{"c","d","beta"},{"e","f","experimental"}};
    auto s=PairMatrixRunner::runAll(pairs);
    C(s.totalPairs==3,"total");P();}

void t4(){T(json_output);
    std::vector<LanguagePair> pairs={{"python","cpp","stable"}};
    auto s=PairMatrixRunner::runAll(pairs);
    auto j=PairMatrixRunner::toJson(s);
    C(j.contains("total"),"has total");P();}

void t5(){T(empty_pairs);
    auto s=PairMatrixRunner::runAll({});
    C(s.totalPairs==0,"empty");P();}

void t6(){T(tier_values);
    LanguagePair lp{"js","ts","experimental"};
    auto r=PairMatrixRunner::runPair(lp);
    C(r.pair.tier=="experimental","tier");P();}

void t7(){T(pass_fail_distinction);
    LanguagePair lp{"rust","cpp","stable"};
    auto r=PairMatrixRunner::runPair(lp);
    C(r.passed==true,"passed");P();}

void t8(){T(all_pass_if_nonempty);
    std::vector<LanguagePair> pairs={{"a","b","stable"},{"c","d","beta"}};
    auto s=PairMatrixRunner::runAll(pairs);
    C(s.passedPairs==2,"all pass");P();}

void t9(){T(failed_count_zero_when_all_pass);
    std::vector<LanguagePair> pairs={{"x","y","stable"}};
    auto s=PairMatrixRunner::runAll(pairs);
    C(s.failedPairs==0,"no failures");P();}

void t10(){T(result_order_matches_input);
    std::vector<LanguagePair> pairs={{"python","cpp","stable"},{"rust","go","beta"}};
    auto s=PairMatrixRunner::runAll(pairs);
    C(s.results[0].pair.source=="python","first");
    C(s.results[1].pair.source=="rust","second");P();}

void t11(){T(status_pass);
    LanguagePair lp{"cpp","rust","beta"};
    auto r=PairMatrixRunner::runPair(lp);
    C(r.status=="pass","status");P();}

void t12(){T(json_has_total_key);
    auto s=PairMatrixRunner::runAll({});
    auto j=PairMatrixRunner::toJson(s);
    C(j["total"]==0,"total key");P();}

int main(){
    std::cout<<"Step 829: PairMatrixRunner\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
