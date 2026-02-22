// Step 701: Trait + impl lowering (10 tests)

#include "rust_ir/RustTraitImplLowering.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static const char* src =
    "trait Draw {\n"
    "  fn draw(&self);\n"
    "}\n"
    "impl Draw for Canvas {\n"
    "  fn draw(&self) {}\n"
    "}\n";

void t1(){T(trait_detected);auto m=RustTraitImplLowering::lower(src);C(m.traits.size()==1,"trait missing");P();}
void t2(){T(trait_name_parsed);auto m=RustTraitImplLowering::lower(src);C(m.traits[0].traitName=="Draw","trait name");P();}
void t3(){T(trait_method_detected);auto m=RustTraitImplLowering::lower(src);C(!m.traits[0].methods.empty(),"method missing");P();}
void t4(){T(impl_detected);auto m=RustTraitImplLowering::lower(src);C(m.impls.size()==1,"impl missing");P();}
void t5(){T(impl_trait_name_parsed);auto m=RustTraitImplLowering::lower(src);C(m.impls[0].traitName=="Draw","impl trait");P();}
void t6(){T(impl_target_parsed);auto m=RustTraitImplLowering::lower(src);C(m.impls[0].forType=="Canvas","impl target");P();}
void t7(){T(json_contains_traits);auto j=RustTraitImplLowering::toJson(RustTraitImplLowering::lower(src));C(j.contains("traits"),"traits key");P();}
void t8(){T(json_contains_impls);auto j=RustTraitImplLowering::toJson(RustTraitImplLowering::lower(src));C(j.contains("impls"),"impls key");P();}
void t9(){T(deterministic);auto a=RustTraitImplLowering::toJson(RustTraitImplLowering::lower(src)).dump();auto b=RustTraitImplLowering::toJson(RustTraitImplLowering::lower(src)).dump();C(a==b,"nondeterministic");P();}
void t10(){T(empty_source);auto m=RustTraitImplLowering::lower("");C(m.traits.empty()&&m.impls.empty(),"expected empty");P();}

int main(){std::cout<<"Step 701: Trait/impl lowering\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
