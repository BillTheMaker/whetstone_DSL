// Step 903: Migration rollback policy (8 tests)
#include "graduation/MigrationRollbackPolicy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(default_action_checkpoint);
    auto pol=MigrationRollbackPolicy::defaultPolicy("p1");
    C(pol.action=="checkpoint","ckpt");P();}
void t2(){T(default_max_retries_three);
    auto pol=MigrationRollbackPolicy::defaultPolicy("p1");
    C(pol.maxRetries==3,"retries");P();}
void t3(){T(strict_action_abort);
    auto pol=MigrationRollbackPolicy::strict("p1");
    C(pol.action=="abort","abort");P();}
void t4(){T(strict_max_retries_zero);
    auto pol=MigrationRollbackPolicy::strict("p1");
    C(pol.maxRetries==0,"zero");P();}
void t5(){T(pair_id_set_default);
    auto pol=MigrationRollbackPolicy::defaultPolicy("myPair");
    C(pol.pairId=="myPair","id");P();}
void t6(){T(pair_id_set_strict);
    auto pol=MigrationRollbackPolicy::strict("myPair");
    C(pol.pairId=="myPair","id");P();}
void t7(){T(to_json_has_action);
    auto pol=MigrationRollbackPolicy::defaultPolicy("p1");
    auto j=MigrationRollbackPolicy::toJson(pol);
    C(j.contains("action"),"json");P();}
void t8(){T(to_json_has_max_retries);
    auto pol=MigrationRollbackPolicy::defaultPolicy("p1");
    auto j=MigrationRollbackPolicy::toJson(pol);
    C(j.contains("max_retries"),"json");P();}

int main(){
    std::cout<<"Step 903: Migration rollback policy\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
