// Step 906: Migration checkpoint store (8 tests)
#include "graduation/MigrationCheckpointStore.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(save_returns_checkpoint);
    MigrationCheckpointStore st;
    auto cp=st.save("p1",0);
    C(!cp.checkpointId.empty(),"id");P();}
void t2(){T(checkpoint_id_sequential);
    MigrationCheckpointStore st;
    auto cp1=st.save("p1",0);
    auto cp2=st.save("p1",1);
    C(cp1.checkpointId=="CP-1"&&cp2.checkpointId=="CP-2","ids");P();}
void t3(){T(has_after_save);
    MigrationCheckpointStore st;
    auto cp=st.save("p1",0);
    C(st.has(cp.checkpointId),"has");P();}
void t4(){T(not_has_unknown);
    MigrationCheckpointStore st;
    C(!st.has("CP-99"),"not has");P();}
void t5(){T(restore_returns_true);
    MigrationCheckpointStore st;
    auto cp=st.save("p1",0);
    C(st.restore(cp.checkpointId),"restore");P();}
void t6(){T(restore_unknown_false);
    MigrationCheckpointStore st;
    C(!st.restore("CP-99"),"false");P();}
void t7(){T(count_increments);
    MigrationCheckpointStore st;
    st.save("p1",0);
    st.save("p1",1);
    C(st.count()==2,"count");P();}
void t8(){T(to_json_has_checkpoint_id);
    MigrationCheckpointStore st;
    auto cp=st.save("p1",0);
    auto j=MigrationCheckpointStore::toJson(cp);
    C(j.contains("checkpoint_id"),"json");P();}

int main(){
    std::cout<<"Step 906: Migration checkpoint store\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
