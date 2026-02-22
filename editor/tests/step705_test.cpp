// Step 705: Macro boundary capture + fallback policy (8 tests)

#include "rust_ir/RustMacroBoundaryPolicy.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static const char* src = "println!(\"ok\"); custom_macro!(x); panic!(\"x\");";

void t1(){T(macros_detected);auto v=RustMacroBoundaryPolicy::analyze(src);C(v.size()>=2,"macros missing");P();}
void t2(){T(known_macro_supported);auto v=RustMacroBoundaryPolicy::analyze(src);bool ok=false;for(auto&m:v)if(m.macroName=="println"&&m.supported)ok=true;C(ok,"println support wrong");P();}
void t3(){T(unknown_macro_review);auto v=RustMacroBoundaryPolicy::analyze(src);bool ok=false;for(auto&m:v)if(m.macroName=="custom_macro"&&!m.supported&&m.fallbackPolicy=="manual_review_required")ok=true;C(ok,"custom macro fallback wrong");P();}
void t4(){T(fallback_inline_for_known);auto v=RustMacroBoundaryPolicy::analyze(src);bool ok=false;for(auto&m:v)if(m.macroName=="panic"&&m.fallbackPolicy=="inline_expand")ok=true;C(ok,"panic fallback wrong");P();}
void t5(){T(json_array);auto j=RustMacroBoundaryPolicy::toJson(RustMacroBoundaryPolicy::analyze(src));C(j.is_array(),"not array");P();}
void t6(){T(json_has_macro_name);auto j=RustMacroBoundaryPolicy::toJson(RustMacroBoundaryPolicy::analyze(src));C(j[0].contains("macroName"),"macroName missing");P();}
void t7(){T(json_has_fallback);auto j=RustMacroBoundaryPolicy::toJson(RustMacroBoundaryPolicy::analyze(src));C(j[0].contains("fallbackPolicy"),"fallback missing");P();}
void t8(){T(deterministic);auto a=RustMacroBoundaryPolicy::toJson(RustMacroBoundaryPolicy::analyze(src)).dump();auto b=RustMacroBoundaryPolicy::toJson(RustMacroBoundaryPolicy::analyze(src)).dump();C(a==b,"nondeterministic");P();}

int main(){std::cout<<"Step 705: Macro boundary policy\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
