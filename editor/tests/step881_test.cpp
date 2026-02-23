// Step 881: Parallel dispatch optimizer (10 tests)
#include "graduation/ParallelDispatchOptimizer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(schedules_up_to_max);
    auto plan=ParallelDispatchOptimizer::optimize({"a","b","c"},2);
    C(plan.scheduledCount==2,"count");P();}
void t2(){T(max_parallelism_stored);
    auto plan=ParallelDispatchOptimizer::optimize({"a","b"},3);
    C(plan.maxParallelism==3,"max");P();}
void t3(){T(slot_count_equals_pairs);
    auto plan=ParallelDispatchOptimizer::optimize({"a","b","c"},3);
    C(plan.slots.size()==3,"slots");P();}
void t4(){T(blocked_pairs_not_scheduled);
    auto plan=ParallelDispatchOptimizer::optimize({"a","b","c"},3,{"b"});
    bool b_scheduled=false;
    for(auto& s:plan.slots) if(s.pairId=="b"&&s.scheduled) b_scheduled=true;
    C(!b_scheduled,"not sched");P();}
void t5(){T(slot_ids_sequential);
    auto plan=ParallelDispatchOptimizer::optimize({"a","b"},2);
    C(plan.slots[0].slotId=="slot-1"&&plan.slots[1].slotId=="slot-2","ids");P();}
void t6(){T(empty_pairs_zero_scheduled);
    auto plan=ParallelDispatchOptimizer::optimize({},2);
    C(plan.scheduledCount==0,"zero");P();}
void t7(){T(more_pairs_than_max);
    auto plan=ParallelDispatchOptimizer::optimize({"a","b","c","d"},2);
    C(plan.scheduledCount==2,"capped");P();}
void t8(){T(all_pairs_if_within_max);
    auto plan=ParallelDispatchOptimizer::optimize({"a","b"},5);
    C(plan.scheduledCount==2,"all");P();}
void t9(){T(to_json_has_scheduled_count);
    auto plan=ParallelDispatchOptimizer::optimize({"a","b"},2);
    auto j=ParallelDispatchOptimizer::toJson(plan);
    C(j.contains("scheduled_count"),"json");P();}
void t10(){T(to_json_slots_array);
    auto plan=ParallelDispatchOptimizer::optimize({"a"},1);
    auto j=ParallelDispatchOptimizer::toJson(plan);
    C(j["slots"].is_array(),"slots");P();}

int main(){
    std::cout<<"Step 881: Parallel dispatch optimizer\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
