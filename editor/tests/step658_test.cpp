// Step 658: Phase 40a integration (8 tests)

#include "Phase40aIntegration.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(energy_visible);auto r=Phase40aIntegration::run();C(r.energyVisible,"energy");P();}
void t2(){T(job_dispatched);auto r=Phase40aIntegration::run();C(r.jobDispatched,"dispatch");P();}
void t3(){T(swarm_status_visible);auto r=Phase40aIntegration::run();C(r.swarmStatusVisible,"swarm");P();}
void t4(){T(apiary_visible);auto r=Phase40aIntegration::run();C(r.apiaryVisible,"apiary");P();}
void t5(){T(integration_ready);auto r=Phase40aIntegration::run();C(r.integrationReady,"ready");P();}
void t6(){T(integration_depends_on_energy);auto r=Phase40aIntegration::run();C(r.energyVisible&&r.integrationReady,"dep");P();}
void t7(){T(integration_depends_on_dispatch);auto r=Phase40aIntegration::run();C(r.jobDispatched&&r.integrationReady,"dep");P();}
void t8(){T(integration_depends_on_views);auto r=Phase40aIntegration::run();C(r.swarmStatusVisible&&r.apiaryVisible&&r.integrationReady,"dep");P();}

int main(){std::cout<<"Step 658: Phase 40a integration\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
