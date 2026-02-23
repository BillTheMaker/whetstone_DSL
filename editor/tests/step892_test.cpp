// Step 892: Runtime-aware lowering extension points (8 tests)
#include "graduation/RuntimeAwareLowering.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(pair_id_set);
    auto pl=RuntimeAwareLowering::plan("p1","jvm11",true);
    C(pl.pairId=="p1","id");P();}
void t2(){T(target_runtime_set);
    auto pl=RuntimeAwareLowering::plan("p1","jvm11",true);
    C(pl.targetRuntime=="jvm11","rt");P();}
void t3(){T(pack_available_flag);
    auto pl=RuntimeAwareLowering::plan("p1","jvm11",true);
    C(pl.runtimePackAvailable,"flag");P();}
void t4(){T(pack_available_two_points);
    auto pl=RuntimeAwareLowering::plan("p1","jvm11",true);
    C(pl.points.size()==2,"two");P();}
void t5(){T(no_pack_one_point);
    auto pl=RuntimeAwareLowering::plan("p1","jvm11",false);
    C(pl.points.size()==1,"one");P();}
void t6(){T(pack_points_active);
    auto pl=RuntimeAwareLowering::plan("p1","jvm11",true);
    C(pl.points[0].active&&pl.points[1].active,"active");P();}
void t7(){T(no_pack_point_inactive);
    auto pl=RuntimeAwareLowering::plan("p1","jvm11",false);
    C(!pl.points[0].active,"inactive");P();}
void t8(){T(to_json_has_extension_points);
    auto pl=RuntimeAwareLowering::plan("p1","jvm11",true);
    auto j=RuntimeAwareLowering::toJson(pl);
    C(j.contains("extension_points")&&j["extension_points"].is_array(),"json");P();}

int main(){
    std::cout<<"Step 892: Runtime-aware lowering extension points\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
