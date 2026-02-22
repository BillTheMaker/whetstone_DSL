// Step 727: Equivalence evidence bundle exporter (8 tests)
#include "equiv/EquivalenceEvidenceBundle.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static EquivalenceEvidenceBundle mk(){auto d=DifferentialExecutionHarness::run({{"v1","a","a",false}});auto p=PropertyEquivalenceRunner::run(4,42);auto f=FuzzDifferentialRunner::run({2,1});return EquivalenceEvidenceBundleModel::build(d,p,f);} 

void t1(){T(bundle_constructable);auto b=mk();C(b.differential.totalCount==1,"d");P();}
void t2(){T(differential_included);auto b=mk();C(b.differential.equivalentCount==1,"d");P();}
void t3(){T(property_included);auto b=mk();C(b.property.success,"p");P();}
void t4(){T(fuzz_included);auto b=mk();C(b.fuzz.size()==2,"f");P();}
void t5(){T(json_shape);auto j=EquivalenceEvidenceBundleModel::toJson(mk());C(j.contains("differential")&&j.contains("property")&&j.contains("fuzz"),"shape");P();}
void t6(){T(machine_readable);auto j=EquivalenceEvidenceBundleModel::toJson(mk());C(j["differential"].is_object()&&j["property"].is_object()&&j["fuzz"].is_array(),"obj");P();}
void t7(){T(fuzz_sorted);auto j=EquivalenceEvidenceBundleModel::toJson(mk());C(j["fuzz"][0].value("seed",9)==1,"sort");P();}
void t8(){T(deterministic);auto a=EquivalenceEvidenceBundleModel::toJson(mk()).dump();auto b=EquivalenceEvidenceBundleModel::toJson(mk()).dump();C(a==b,"det");P();}

int main(){std::cout<<"Step 727: EquivalenceEvidenceBundle\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
