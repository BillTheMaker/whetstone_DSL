// Step 887: Upgrade queue observability panel model (8 tests)
#include "graduation/UpgradeQueueObservabilityPanel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(snapshot_id_set);
    auto s=UpgradeQueueObservabilityPanel::snapshot("snap-1",10,5,1,4,0.8f);
    C(s.snapshotId=="snap-1","id");P();}
void t2(){T(healthy_label);
    auto s=UpgradeQueueObservabilityPanel::snapshot("s",10,5,0,5,0.9f);
    C(s.healthLabel=="healthy","healthy");P();}
void t3(){T(degraded_label);
    auto s=UpgradeQueueObservabilityPanel::snapshot("s",10,5,1,4,0.5f);
    C(s.healthLabel=="degraded","degraded");P();}
void t4(){T(critical_label);
    auto s=UpgradeQueueObservabilityPanel::snapshot("s",10,5,2,3,0.2f);
    C(s.healthLabel=="critical","critical");P();}
void t5(){T(queued_stored);
    auto s=UpgradeQueueObservabilityPanel::snapshot("s",15,5,1,9,0.8f);
    C(s.totalQueued==15,"queued");P();}
void t6(){T(completed_stored);
    auto s=UpgradeQueueObservabilityPanel::snapshot("s",10,5,1,4,0.8f);
    C(s.totalCompleted==4,"completed");P();}
void t7(){T(health_score_stored);
    auto s=UpgradeQueueObservabilityPanel::snapshot("s",10,5,1,4,0.75f);
    C(s.healthScore==0.75f,"score");P();}
void t8(){T(to_json_has_health_label);
    auto s=UpgradeQueueObservabilityPanel::snapshot("s",10,5,0,5,0.9f);
    auto j=UpgradeQueueObservabilityPanel::toJson(s);
    C(j.contains("health_label"),"json");P();}

int main(){
    std::cout<<"Step 887: Upgrade queue observability panel\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
