// Step 838: HistoricalRegressionChecker tests (8 tests)
#include "graduation/HistoricalRegressionChecker.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(empty_not_clean);
    auto r=HistoricalRegressionChecker::run({});
    C(!r.clean,"not clean");P();}

void t2(){T(all_pass_clean);
    std::vector<RegressionCheckItem> items={{"C1","desc",true},{"C2","desc",true}};
    auto r=HistoricalRegressionChecker::run(items);
    C(r.clean,"clean");P();}

void t3(){T(partial_not_clean);
    std::vector<RegressionCheckItem> items={{"C1","desc",true},{"C2","desc",false}};
    auto r=HistoricalRegressionChecker::run(items);
    C(!r.clean,"not clean");P();}

void t4(){T(totalChecks_count);
    std::vector<RegressionCheckItem> items={{"C1","d",true},{"C2","d",false}};
    auto r=HistoricalRegressionChecker::run(items);
    C(r.totalChecks==2,"totalChecks");P();}

void t5(){T(passedChecks_count);
    std::vector<RegressionCheckItem> items={{"C1","d",true},{"C2","d",false}};
    auto r=HistoricalRegressionChecker::run(items);
    C(r.passedChecks==1,"passedChecks");P();}

void t6(){T(items_preserved);
    std::vector<RegressionCheckItem> items={{"C1","d",true}};
    auto r=HistoricalRegressionChecker::run(items);
    C(r.items.size()==1,"items size");P();}

void t7(){T(toJson);
    auto r=HistoricalRegressionChecker::run({});
    auto j=HistoricalRegressionChecker::toJson(r);
    C(j.contains("total"),"total");C(j.contains("items"),"items");P();}

void t8(){T(toJson_has_clean_key);
    auto r=HistoricalRegressionChecker::run({});
    auto j=HistoricalRegressionChecker::toJson(r);
    C(j.contains("clean"),"clean key");P();}

int main(){
    std::cout<<"Step 838: HistoricalRegressionChecker\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
