// Step 655: Job status subscriber panel (12 tests)

#include "SwarmStatusPanelModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static std::vector<SwarmJobStatusItem> sample(){
return {{"j1","nodeA",0,SwarmJobState::Pending},{"j2","nodeB",12,SwarmJobState::Running},{"j3","nodeC",30,SwarmJobState::Completed},{"j4","nodeD",5,SwarmJobState::Failed}};}

void t1(){T(summary_counts_pending);auto s=SwarmStatusPanelModel::summarize(sample());C(s.pending==1,"pending");P();}
void t2(){T(summary_counts_running);auto s=SwarmStatusPanelModel::summarize(sample());C(s.running==1,"running");P();}
void t3(){T(summary_counts_completed);auto s=SwarmStatusPanelModel::summarize(sample());C(s.completed==1,"completed");P();}
void t4(){T(summary_counts_failed);auto s=SwarmStatusPanelModel::summarize(sample());C(s.failed==1,"failed");P();}
void t5(){T(label_combines_job_and_node);C(SwarmStatusPanelModel::label(sample()[0])=="j1@nodeA","label");P();}
void t6(){T(state_text_pending);C(SwarmStatusPanelModel::stateText(SwarmJobState::Pending)=="pending","text");P();}
void t7(){T(state_text_running);C(SwarmStatusPanelModel::stateText(SwarmJobState::Running)=="running","text");P();}
void t8(){T(state_text_completed);C(SwarmStatusPanelModel::stateText(SwarmJobState::Completed)=="completed","text");P();}
void t9(){T(state_text_failed);C(SwarmStatusPanelModel::stateText(SwarmJobState::Failed)=="failed","text");P();}
void t10(){T(empty_summary_all_zero);auto s=SwarmStatusPanelModel::summarize({});C(s.pending==0&&s.running==0&&s.completed==0&&s.failed==0,"empty");P();}
void t11(){T(running_duration_preserved);auto i=sample()[1];C(i.runningSeconds==12,"duration");P();}
void t12(){T(multi_pending_counts_correctly);auto v=sample();v.push_back({"j5","nodeE",0,SwarmJobState::Pending});auto s=SwarmStatusPanelModel::summarize(v);C(s.pending==2,"multi");P();}

int main(){std::cout<<"Step 655: Swarm status panel\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
