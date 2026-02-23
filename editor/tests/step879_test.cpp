// Step 879: Pair-upgrade queue model (12 tests)
#include "graduation/PairUpgradeQueue.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(enqueue_valid);
    PairUpgradeQueue q;
    C(q.enqueue({"E-1","py->cpp","critical",4,true}),"enqueue");P();}
void t2(){T(enqueue_empty_pair_fails);
    PairUpgradeQueue q; std::string err;
    C(!q.enqueue({"E-1","","critical",4,true},&err)&&err=="pair_id_missing","fail");P();}
void t3(){T(enqueue_empty_id_fails);
    PairUpgradeQueue q; std::string err;
    C(!q.enqueue({"","py->cpp","critical",4,true},&err)&&err=="entry_id_missing","fail");P();}
void t4(){T(active_count_increments);
    PairUpgradeQueue q;
    q.enqueue({"E-1","py->cpp","critical",4,true});
    q.enqueue({"E-2","rust->go","coverage",2,true});
    C(q.activeCount()==2,"count");P();}
void t5(){T(dequeue_removes);
    PairUpgradeQueue q;
    q.enqueue({"E-1","py->cpp","critical",4,true});
    C(q.dequeue("E-1"),"dequeue");P();}
void t6(){T(active_count_after_dequeue);
    PairUpgradeQueue q;
    q.enqueue({"E-1","py->cpp","critical",4,true});
    q.dequeue("E-1");
    C(q.activeCount()==0,"count 0");P();}
void t7(){T(dequeue_unknown_fails);
    PairUpgradeQueue q;
    C(!q.dequeue("NONEXISTENT"),"fail");P();}
void t8(){T(get_active_sorted_by_priority);
    PairUpgradeQueue q;
    q.enqueue({"E-1","py->cpp","experimental",1,true});
    q.enqueue({"E-2","rust->go","critical",4,true});
    auto a=q.getActive();
    C(a[0].priorityLevel==4,"sorted");P();}
void t9(){T(get_active_excludes_dequeued);
    PairUpgradeQueue q;
    q.enqueue({"E-1","py->cpp","critical",4,true});
    q.enqueue({"E-2","rust->go","coverage",2,true});
    q.dequeue("E-1");
    auto a=q.getActive();
    C(a.size()==1,"size");P();}
void t10(){T(to_json_has_entry_id);
    UpgradeQueueEntry e{"E-1","py->cpp","critical",4,true};
    auto j=PairUpgradeQueue::toJson(e);
    C(j.contains("entry_id"),"json");P();}
void t11(){T(to_json_has_priority);
    UpgradeQueueEntry e{"E-1","py->cpp","critical",4,true};
    auto j=PairUpgradeQueue::toJson(e);
    C(j["priority"]=="critical","priority");P();}
void t12(){T(empty_queue_active_zero);
    PairUpgradeQueue q;
    C(q.activeCount()==0,"zero");P();}

int main(){
    std::cout<<"Step 879: Pair-upgrade queue model\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
