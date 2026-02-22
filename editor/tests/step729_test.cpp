// Step 729: Security scan orchestrator for generated targets (12 tests)
#include "gates/SecurityScanOrchestrator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static std::vector<SecurityFinding> sample(){return {{"2","low","b.cpp"},{"1","high","a.cpp"}};}
void t1(){T(run_non_empty);auto r=SecurityScanOrchestrator::run(sample());C(r.findings.size()==2,"size");P();}
void t2(){T(high_count_detected);auto r=SecurityScanOrchestrator::run(sample());C(r.highCount==1,"high");P();}
void t3(){T(sorted_by_severity_then_file);auto r=SecurityScanOrchestrator::run(sample());C(r.findings[0].severity=="high","sort");P();}
void t4(){T(empty_supported);auto r=SecurityScanOrchestrator::run({});C(r.findings.empty()&&r.highCount==0,"empty");P();}
void t5(){T(multiple_high_count);auto r=SecurityScanOrchestrator::run({{"1","high","a"},{"2","high","b"}});C(r.highCount==2,"high2");P();}
void t6(){T(low_not_counted);auto r=SecurityScanOrchestrator::run({{"1","low","a"}});C(r.highCount==0,"low");P();}
void t7(){T(json_shape);auto j=SecurityScanOrchestrator::toJson(SecurityScanOrchestrator::run(sample()));C(j.contains("findings")&&j.contains("high_count"),"shape");P();}
void t8(){T(json_findings_array);auto j=SecurityScanOrchestrator::toJson(SecurityScanOrchestrator::run(sample()));C(j["findings"].is_array(),"arr");P();}
void t9(){T(json_finding_fields);auto j=SecurityScanOrchestrator::toJson(SecurityScanOrchestrator::run(sample()));C(j["findings"][0].contains("id")&&j["findings"][0].contains("severity")&&j["findings"][0].contains("file"),"fields");P();}
void t10(){T(machine_readable);auto j=SecurityScanOrchestrator::toJson(SecurityScanOrchestrator::run(sample()));C(j.is_object(),"obj");P();}
void t11(){T(stable_size);auto r=SecurityScanOrchestrator::run(sample());C(r.findings.size()==2,"stable");P();}
void t12(){T(deterministic);auto a=SecurityScanOrchestrator::toJson(SecurityScanOrchestrator::run(sample())).dump();auto b=SecurityScanOrchestrator::toJson(SecurityScanOrchestrator::run(sample())).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 729: SecurityScanOrchestrator\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
