// Step 732: Perf benchmark harness contract (10 tests)
#include "gates/PerfBenchmarkContract.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static std::vector<PerfBenchmarkCase> sample(){return {{"b",10,11},{"a",10,9}};}
void t1(){T(normalize_sorts);auto c=PerfBenchmarkContract::normalize(sample());C(c[0].name=="a","sort");P();}
void t2(){T(values_preserved);auto c=PerfBenchmarkContract::normalize(sample());C(c[0].baselineMs==10,"val");P();}
void t3(){T(empty_supported);auto c=PerfBenchmarkContract::normalize({});C(c.empty(),"empty");P();}
void t4(){T(to_json_array);auto j=PerfBenchmarkContract::toJson(PerfBenchmarkContract::normalize(sample()));C(j.is_array(),"arr");P();}
void t5(){T(json_case_fields);auto j=PerfBenchmarkContract::toJson(PerfBenchmarkContract::normalize(sample()));C(j[0].contains("name")&&j[0].contains("baseline_ms")&&j[0].contains("target_ms"),"fields");P();}
void t6(){T(machine_readable);auto j=PerfBenchmarkContract::toJson(PerfBenchmarkContract::normalize(sample()));C(j[0].is_object(),"obj");P();}
void t7(){T(stable_count);auto c=PerfBenchmarkContract::normalize(sample());C(c.size()==2,"count");P();}
void t8(){T(non_negative_ms_supported);auto c=PerfBenchmarkContract::normalize({{"a",0,0}});C(c[0].baselineMs==0,"zero");P();}
void t9(){T(deterministic_json);auto a=PerfBenchmarkContract::toJson(PerfBenchmarkContract::normalize(sample())).dump();auto b=PerfBenchmarkContract::toJson(PerfBenchmarkContract::normalize(sample())).dump();C(a==b,"det");P();}
void t10(){T(deterministic_normalize);auto a=PerfBenchmarkContract::normalize(sample());auto b=PerfBenchmarkContract::normalize(sample());C(a[0].name==b[0].name&&a[1].name==b[1].name,"det");P();}
int main(){std::cout<<"Step 732: PerfBenchmarkContract\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
