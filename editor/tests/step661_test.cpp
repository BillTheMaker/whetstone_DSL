// Step 661: Pilot queue panel (12 tests)

#include "PilotQueuePanelModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(needs_review_when_escalate_true);PilotQueueItem i{"j","s",true,0.1};C(PilotQueuePanelModel::needsHumanReview(i),"review");P();}
void t2(){T(needs_review_when_ambiguity_above_threshold);PilotQueueItem i{"j","s",false,0.8};C(PilotQueuePanelModel::needsHumanReview(i),"review");P();}
void t3(){T(no_review_when_below_threshold_and_not_escalated);PilotQueueItem i{"j","s",false,0.2};C(!PilotQueuePanelModel::needsHumanReview(i),"no review");P();}
void t4(){T(approve_sets_decision);PilotQueueItem i;PilotQueuePanelModel::approve(&i);C(i.decision==PilotDecision::Approved,"approve");P();}
void t5(){T(reject_sets_decision);PilotQueueItem i;PilotQueuePanelModel::reject(&i);C(i.decision==PilotDecision::Rejected,"reject");P();}
void t6(){T(modify_sets_decision);PilotQueueItem i;PilotQueuePanelModel::modify(&i,"note");C(i.decision==PilotDecision::Modified,"modify");P();}
void t7(){T(modify_sets_strategy_note);PilotQueueItem i;PilotQueuePanelModel::modify(&i,"use safer path");C(i.strategyNote=="use safer path","note");P();}
void t8(){T(approve_handles_null);PilotQueuePanelModel::approve(nullptr);P();}
void t9(){T(reject_handles_null);PilotQueuePanelModel::reject(nullptr);P();}
void t10(){T(modify_handles_null);PilotQueuePanelModel::modify(nullptr,"x");P();}
void t11(){T(custom_threshold_respected);PilotQueueItem i{"j","s",false,0.6};C(!PilotQueuePanelModel::needsHumanReview(i,0.7),"threshold");P();}
void t12(){T(threshold_strictly_greater);PilotQueueItem i{"j","s",false,0.5};C(!PilotQueuePanelModel::needsHumanReview(i,0.5),"strict");P();}

int main(){std::cout<<"Step 661: Pilot queue panel\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
