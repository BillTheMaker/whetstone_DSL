// Step 817: Modernization dossier generator (8 tests)
#include "legacy_ingestion/ModernizationDossier.h"
#include "legacy_ingestion/MigrationReadiness.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(entries_two);auto s=MigrationReadiness::evaluate(0.8,6);auto d=ModernizationDossier::produce(s);C(d.size()==2,"size");P();}
void t2(){T(status_ready);auto s=MigrationReadiness::evaluate(0.9,6);auto d=ModernizationDossier::produce(s);C(d[0].status=="ready","status");P();}
void t3(){T(api_ready_when_complete);auto s=MigrationReadiness::evaluate(0.9,10);auto d=ModernizationDossier::produce(s);C(d[1].status=="ready","status");P();}
void t4(){T(api_review_if_incomplete);auto s=MigrationReadiness::evaluate(0.4,1);auto d=ModernizationDossier::produce(s);C(d[1].status=="review","status");P();}
void t5(){T(json_shape);auto j=ModernizationDossier::toJson(ModernizationDossier::produce(MigrationReadiness::evaluate(0.9,6))[0]);C(j.contains("area"),"shape");P();}
void t6(){T(machine_readable);auto j=ModernizationDossier::toJson(ModernizationDossier::produce(MigrationReadiness::evaluate(0.9,6))[0]);C(j.is_object(),"obj");P();}
void t7(){T(deterministic);auto a=ModernizationDossier::toJson(ModernizationDossier::produce(MigrationReadiness::evaluate(0.9,6))[0]).dump();auto b=ModernizationDossier::toJson(ModernizationDossier::produce(MigrationReadiness::evaluate(0.9,6))[0]).dump();C(a==b,"det");P();}
void t8(){T(confidence_flow);auto s=MigrationReadiness::evaluate(0.7,4);auto d=ModernizationDossier::produce(s);C(d[0].confidence==s.confidence,"conf");P();}
int main(){std::cout<<"Step 817: ModernizationDossier\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
