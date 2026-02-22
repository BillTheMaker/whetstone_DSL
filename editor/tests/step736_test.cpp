// Step 736: Migration acceptance contract wiring (8 tests)
#include "MigrationAcceptanceContract.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static MigrationGateThresholds th(){MigrationGateThresholds t; t.maxHighSeverityFindings=0; t.maxPerfRegressionPct=10.0f; return t;}
void t1(){T(pass_clean);PortingGateEvidence e; e.sanitizerPass=true;auto r=MigrationAcceptanceContract::evaluatePortingGates(e,th());C(r.pass,"pass");P();}
void t2(){T(fail_security_without_waiver);PortingGateEvidence e; e.securityHighFindings=1; e.sanitizerPass=true;auto r=MigrationAcceptanceContract::evaluatePortingGates(e,th());C(!r.pass,"fail");P();}
void t3(){T(warn_security_with_waiver);PortingGateEvidence e; e.securityHighFindings=1; e.hasSecurityWaiver=true; e.sanitizerPass=true;auto r=MigrationAcceptanceContract::evaluatePortingGates(e,th());C(r.pass&&!r.warningGates.empty(),"warn");P();}
void t4(){T(fail_sanitizer);PortingGateEvidence e; e.sanitizerPass=false;auto r=MigrationAcceptanceContract::evaluatePortingGates(e,th());C(!r.pass,"fail");P();}
void t5(){T(warn_supply_chain);PortingGateEvidence e; e.sanitizerPass=true; e.supplyChainHighFindings=1;auto r=MigrationAcceptanceContract::evaluatePortingGates(e,th());C(r.pass&&!r.warningGates.empty(),"warn");P();}
void t6(){T(fail_perf_without_waiver);PortingGateEvidence e; e.sanitizerPass=true; e.worstPerfRegressionPct=20.0;auto r=MigrationAcceptanceContract::evaluatePortingGates(e,th());C(!r.pass,"fail");P();}
void t7(){T(warn_perf_with_waiver);PortingGateEvidence e; e.sanitizerPass=true; e.worstPerfRegressionPct=20.0; e.hasPerfWaiver=true;auto r=MigrationAcceptanceContract::evaluatePortingGates(e,th());C(r.pass&&!r.warningGates.empty(),"warn");P();}
void t8(){T(deterministic);PortingGateEvidence e; e.sanitizerPass=true;auto a=MigrationAcceptanceContract::evaluatePortingGates(e,th());auto b=MigrationAcceptanceContract::evaluatePortingGates(e,th());C(a.summary==b.summary&&a.pass==b.pass,"det");P();}
int main(){std::cout<<"Step 736: MigrationAcceptanceContract wiring\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
