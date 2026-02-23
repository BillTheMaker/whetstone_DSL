// Step 896: Runtime profile store (8 tests)
#include "graduation/RuntimeProfileStore.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(set_and_get);
    RuntimeProfileStore s;
    s.set({"p1","jvm11","11"});
    auto* pr=s.get("p1");
    C(pr&&pr->runtimeId=="jvm11","get");P();}
void t2(){T(get_unknown_null);
    RuntimeProfileStore s;
    C(s.get("unknown")==nullptr,"null");P();}
void t3(){T(has_after_set);
    RuntimeProfileStore s;
    s.set({"p1","jvm11","11"});
    C(s.has("p1"),"has");P();}
void t4(){T(not_has_unknown);
    RuntimeProfileStore s;
    C(!s.has("unknown"),"not has");P();}
void t5(){T(set_overwrites);
    RuntimeProfileStore s;
    s.set({"p1","jvm11","11"});
    s.set({"p1","cpython3","3.11"});
    C(s.get("p1")->runtimeId=="cpython3","overwrite");P();}
void t6(){T(count_correct);
    RuntimeProfileStore s;
    s.set({"p1","jvm11","11"});
    s.set({"p2","cpython3","3.11"});
    C(s.count()==2,"count");P();}
void t7(){T(list_returns_ids);
    RuntimeProfileStore s;
    s.set({"p1","jvm11","11"});
    C(s.list().size()==1,"list");P();}
void t8(){T(clear_removes_all);
    RuntimeProfileStore s;
    s.set({"p1","jvm11","11"});
    s.clear();
    C(s.count()==0,"clear");P();}

int main(){
    std::cout<<"Step 896: Runtime profile store\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
