// Step 702: Generic/monomorphization intent model (10 tests)

#include "rust_ir/RustGenericIntentModel.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static const char* src =
    "fn map<T, U>(x: T) -> U where U: Default { U::default() }\n"
    "struct Boxed<T> { value: T }\n";

void t1(){T(generic_fn_detected);auto v=RustGenericIntentModel::lower(src);bool ok=false;for(auto&i:v)if(i.owner=="map")ok=true;C(ok,"map missing");P();}
void t2(){T(generic_struct_detected);auto v=RustGenericIntentModel::lower(src);bool ok=false;for(auto&i:v)if(i.owner=="Boxed")ok=true;C(ok,"Boxed missing");P();}
void t3(){T(params_parsed);auto v=RustGenericIntentModel::lower(src);bool ok=false;for(auto&i:v)if(i.owner=="map"&&i.params.size()==2)ok=true;C(ok,"params wrong");P();}
void t4(){T(monomorphization_hint);auto v=RustGenericIntentModel::lower(src);bool ok=false;for(auto&i:v)if(i.owner=="map"&&i.monomorphizedHint)ok=true;C(ok,"hint missing");P();}
void t5(){T(json_export_array);auto j=RustGenericIntentModel::toJson(RustGenericIntentModel::lower(src));C(j.is_array(),"not array");P();}
void t6(){T(json_has_owner);auto j=RustGenericIntentModel::toJson(RustGenericIntentModel::lower(src));C(j[0].contains("owner"),"owner missing");P();}
void t7(){T(json_has_params);auto j=RustGenericIntentModel::toJson(RustGenericIntentModel::lower(src));C(j[0].contains("params"),"params missing");P();}
void t8(){T(non_generic_ignored);auto v=RustGenericIntentModel::lower("fn plain(x:i32)->i32{x}\n");C(v.empty(),"plain should be ignored");P();}
void t9(){T(deterministic);auto a=RustGenericIntentModel::toJson(RustGenericIntentModel::lower(src)).dump();auto b=RustGenericIntentModel::toJson(RustGenericIntentModel::lower(src)).dump();C(a==b,"nondeterministic");P();}
void t10(){T(sorted);auto v=RustGenericIntentModel::lower(src);C(v.size()<2 || v[0].owner<=v[1].owner,"unsorted");P();}

int main(){std::cout<<"Step 702: Generic intent model\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
