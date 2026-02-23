// Step 905: Migration progress tracker (8 tests)
#include "graduation/MigrationProgressTracker.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(start_status_pending);
    MigrationProgressTracker tr;
    tr.start("p1",5);
    auto* pg=tr.get("p1");
    C(pg&&pg->status=="pending","pending");P();}
void t2(){T(advance_increments_completed);
    MigrationProgressTracker tr;
    tr.start("p1",5);
    tr.advance("p1");
    auto* pg=tr.get("p1");
    C(pg&&pg->completedSteps==1,"completed");P();}
void t3(){T(complete_sets_status);
    MigrationProgressTracker tr;
    tr.start("p1",5);
    tr.complete("p1");
    auto* pg=tr.get("p1");
    C(pg&&pg->status=="complete","complete");P();}
void t4(){T(fail_sets_status);
    MigrationProgressTracker tr;
    tr.start("p1",5);
    tr.fail("p1");
    auto* pg=tr.get("p1");
    C(pg&&pg->status=="failed","failed");P();}
void t5(){T(get_unknown_null);
    MigrationProgressTracker tr;
    C(tr.get("unknown")==nullptr,"null");P();}
void t6(){T(percent_complete_zero);
    MigrationProgressTracker tr;
    tr.start("p1",4);
    C(tr.percentComplete("p1")==0.0f,"zero");P();}
void t7(){T(percent_complete_half);
    MigrationProgressTracker tr;
    tr.start("p1",4);
    tr.advance("p1");
    tr.advance("p1");
    C(tr.percentComplete("p1")==50.0f,"half");P();}
void t8(){T(to_json_has_status);
    MigrationProgressTracker tr;
    tr.start("p1",5);
    auto pg=tr.get("p1");
    auto j=MigrationProgressTracker::toJson(*pg);
    C(j.contains("status"),"json");P();}

int main(){
    std::cout<<"Step 905: Migration progress tracker\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
