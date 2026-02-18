// Step 648: Phase 39a integration (8 tests)

#include "Phase39aIntegration.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(config_loaded);auto r=Phase39aIntegration::run();C(r.configLoaded,"config");P();}
void t2(){T(cmake_generated);auto r=Phase39aIntegration::run();C(r.cmakeGenerated,"cmake");P();}
void t3(){T(test_run_triggered);auto r=Phase39aIntegration::run();C(r.testRunTriggered,"trigger");P();}
void t4(){T(test_run_complete);auto r=Phase39aIntegration::run();C(r.testRunComplete,"complete");P();}
void t5(){T(mutation_blocked_for_live_file);auto r=Phase39aIntegration::run();C(r.mutationBlocked,"blocked");P();}
void t6(){T(safety_reason_present);auto r=Phase39aIntegration::run();C(r.safetyReasonPresent,"reason");P();}
void t7(){T(self_hosting_ready);auto r=Phase39aIntegration::run();C(r.selfHostingReady,"self-host");P();}
void t8(){T(integration_ready_signal);auto r=Phase39aIntegration::run();C(r.integrationReady,"integration");P();}

int main(){std::cout<<"Step 648: Phase 39a integration\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
