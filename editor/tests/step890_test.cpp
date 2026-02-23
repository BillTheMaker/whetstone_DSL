// Step 890: Pack loader and version manager (10 tests)
#include "graduation/RuntimePackLoader.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(load_valid_pack);
    RuntimePackLoader ldr;
    auto pk=RuntimeSemanticsPackSchema::make("jvm11","11");
    C(ldr.load(pk),"load");P();}
void t2(){T(is_loaded_after_load);
    RuntimePackLoader ldr;
    ldr.load(RuntimeSemanticsPackSchema::make("jvm11","11"));
    C(ldr.isLoaded("jvm11"),"loaded");P();}
void t3(){T(not_loaded_unknown);
    RuntimePackLoader ldr;
    C(!ldr.isLoaded("unknown"),"not loaded");P();}
void t4(){T(get_returns_pack);
    RuntimePackLoader ldr;
    ldr.load(RuntimeSemanticsPackSchema::make("jvm11","11"));
    C(ldr.get("jvm11")!=nullptr,"get");P();}
void t5(){T(get_unknown_null);
    RuntimePackLoader ldr;
    C(ldr.get("unknown")==nullptr,"null");P();}
void t6(){T(loaded_count);
    RuntimePackLoader ldr;
    ldr.load(RuntimeSemanticsPackSchema::make("jvm11","11"));
    ldr.load(RuntimeSemanticsPackSchema::make("cpython3","3.11"));
    C(ldr.loadedCount()==2,"count");P();}
void t7(){T(list_loaded);
    RuntimePackLoader ldr;
    ldr.load(RuntimeSemanticsPackSchema::make("jvm11","11"));
    auto list=ldr.listLoaded();
    C(list.size()==1,"size");P();}
void t8(){T(unload_removes);
    RuntimePackLoader ldr;
    ldr.load(RuntimeSemanticsPackSchema::make("jvm11","11"));
    C(ldr.unload("jvm11")&&!ldr.isLoaded("jvm11"),"unload");P();}
void t9(){T(load_invalid_fails);
    RuntimePackLoader ldr;
    RuntimeSemanticsPack bad; // empty runtimeId
    C(!ldr.load(bad),"fail");P();}
void t10(){T(count_zero_initially);
    RuntimePackLoader ldr;
    C(ldr.loadedCount()==0,"zero");P();}

int main(){
    std::cout<<"Step 890: Pack loader and version manager\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
