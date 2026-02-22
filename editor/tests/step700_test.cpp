// Step 700: Lifetime region lowering (10 tests)

#include "rust_ir/RustLifetimeRegionLowering.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static const char* src = "fn f<'a>(x: &'a i32) -> &'a i32 { x }\nstruct S<'b>{x:&'b i32}\n";

void t1(){T(extract_non_empty);auto v=RustLifetimeRegionLowering::lower(src);C(!v.empty(),"empty");P();}
void t2(){T(contains_a);auto v=RustLifetimeRegionLowering::lower(src);bool ok=false;for(auto&r:v)if(r.name=="'a")ok=true;C(ok,"'a missing");P();}
void t3(){T(contains_b);auto v=RustLifetimeRegionLowering::lower(src);bool ok=false;for(auto&r:v)if(r.name=="'b")ok=true;C(ok,"'b missing");P();}
void t4(){T(no_duplicates);auto v=RustLifetimeRegionLowering::lower(src);int ca=0;for(auto&r:v)if(r.name=="'a")++ca;C(ca==1,"duplicate lifetime");P();}
void t5(){T(scope_present);auto v=RustLifetimeRegionLowering::lower(src);C(!v[0].scope.empty(),"scope empty");P();}
void t6(){T(json_export_array);auto j=RustLifetimeRegionLowering::toJson(RustLifetimeRegionLowering::lower(src));C(j.is_array(),"not array");P();}
void t7(){T(json_export_has_name);auto j=RustLifetimeRegionLowering::toJson(RustLifetimeRegionLowering::lower(src));C(j[0].contains("name"),"name missing");P();}
void t8(){T(empty_source_empty_result);auto v=RustLifetimeRegionLowering::lower("");C(v.empty(),"expected empty");P();}
void t9(){T(deterministic);auto a=RustLifetimeRegionLowering::toJson(RustLifetimeRegionLowering::lower(src)).dump();auto b=RustLifetimeRegionLowering::toJson(RustLifetimeRegionLowering::lower(src)).dump();C(a==b,"nondeterministic");P();}
void t10(){T(sorted_names);auto v=RustLifetimeRegionLowering::lower(src);C(v.size()<2 || v[0].name<=v[1].name,"unsorted");P();}

int main(){std::cout<<"Step 700: Lifetime lowering\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
