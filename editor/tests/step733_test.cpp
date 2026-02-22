// Step 733: Baseline-vs-target perf comparator (10 tests)
#include "gates/PerfComparator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static std::vector<PerfBenchmarkCase> sample(){return {{"a",10,11},{"b",20,20}};}
void t1(){T(regression_computed);auto r=PerfComparator::compare(sample(),20.0);C(r.worstRegressionPct>0.0,"pct");P();}
void t2(){T(pass_within_threshold);auto r=PerfComparator::compare(sample(),20.0);C(r.pass,"pass");P();}
void t3(){T(fail_above_threshold);auto r=PerfComparator::compare(sample(),5.0);C(!r.pass,"fail");P();}
void t4(){T(skip_zero_baseline);auto r=PerfComparator::compare({{"a",0,10}},5.0);C(r.worstRegressionPct==0.0,"skip");P();}
void t5(){T(empty_supported);auto r=PerfComparator::compare({},5.0);C(r.pass&&r.worstRegressionPct==0.0,"empty");P();}
void t6(){T(json_shape);auto j=PerfComparator::toJson(PerfComparator::compare(sample(),20.0));C(j.contains("worst_regression_pct")&&j.contains("pass"),"shape");P();}
void t7(){T(machine_readable);auto j=PerfComparator::toJson(PerfComparator::compare(sample(),20.0));C(j.is_object(),"obj");P();}
void t8(){T(non_negative_regression);auto r=PerfComparator::compare({{"a",10,9}},20.0);C(r.worstRegressionPct>=0.0,"nonneg");P();}
void t9(){T(deterministic);auto a=PerfComparator::toJson(PerfComparator::compare(sample(),20.0)).dump();auto b=PerfComparator::toJson(PerfComparator::compare(sample(),20.0)).dump();C(a==b,"det");P();}
void t10(){T(worst_of_multiple);auto r=PerfComparator::compare({{"a",10,12},{"b",10,11}},50.0);C(r.worstRegressionPct>15.0,"worst");P();}
int main(){std::cout<<"Step 733: PerfComparator\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
