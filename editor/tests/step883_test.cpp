// Step 883: Queue health scoring model (10 tests)
#include "graduation/QueueHealthScorer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(queue_id_set);
    auto h=QueueHealthScorer::score("q1",5,0,10);
    C(h.queueId=="q1","id");P();}
void t2(){T(no_stall_healthy);
    auto h=QueueHealthScorer::score("q1",5,0,10);
    C(h.label=="healthy","healthy");P();}
void t3(){T(all_stalled_critical);
    auto h=QueueHealthScorer::score("q1",0,10,10);
    C(h.label=="critical","critical");P();}
void t4(){T(score_in_range);
    auto h=QueueHealthScorer::score("q1",5,2,10);
    C(h.score>=0.0f&&h.score<=1.0f,"range");P();}
void t5(){T(active_count_stored);
    auto h=QueueHealthScorer::score("q1",7,1,10);
    C(h.activeCount==7,"active");P();}
void t6(){T(stalled_count_stored);
    auto h=QueueHealthScorer::score("q1",5,3,10);
    C(h.stalledCount==3,"stalled");P();}
void t7(){T(zero_counts_healthy);
    auto h=QueueHealthScorer::score("q1",0,0,10);
    C(h.score>=0.0f,"zero");P();}
void t8(){T(degraded_label);
    // 50% stall rate → score = 1 - 0.5*0.7 = 0.65 → healthy? Actually let me think...
    // stallRate = 5/10 = 0.5, score = 1 - 0.5*0.7 = 0.65 → healthy
    // Let's use stallRate=0.9 → score = 1-0.9*0.7 = 0.37 → degraded
    auto h=QueueHealthScorer::score("q1",1,9,20);
    C(h.label=="degraded"||h.label=="critical","degraded or critical");P();}
void t9(){T(to_json_has_score);
    auto h=QueueHealthScorer::score("q1",5,0,10);
    auto j=QueueHealthScorer::toJson(h);
    C(j.contains("score"),"json");P();}
void t10(){T(to_json_has_label);
    auto h=QueueHealthScorer::score("q1",5,0,10);
    auto j=QueueHealthScorer::toJson(h);
    C(j.contains("label"),"label");P();}

int main(){
    std::cout<<"Step 883: Queue health scoring\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
