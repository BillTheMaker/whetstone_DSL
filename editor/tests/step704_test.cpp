// Step 704: Async/await state intent lowering (10 tests)

#include "rust_ir/RustAsyncIntentLowering.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static const char* src =
    "async fn run() {\n"
    "  io().await;\n"
    "  next().await;\n"
    "}\n";

void t1(){T(async_fn_detected);auto v=RustAsyncIntentLowering::lower(src);C(v.size()==1,"async fn missing");P();}
void t2(){T(name_parsed);auto v=RustAsyncIntentLowering::lower(src);C(v[0].functionName=="run","name wrong");P();}
void t3(){T(await_count_detected);auto v=RustAsyncIntentLowering::lower(src);C(v[0].awaitCount==2,"await count wrong");P();}
void t4(){T(returns_future_true);auto v=RustAsyncIntentLowering::lower(src);C(v[0].returnsFuture,"returnsFuture false");P();}
void t5(){T(non_async_ignored);auto v=RustAsyncIntentLowering::lower("fn x(){ y().await; }");C(v.empty(),"non-async should be ignored");P();}
void t6(){T(json_is_array);auto j=RustAsyncIntentLowering::toJson(RustAsyncIntentLowering::lower(src));C(j.is_array(),"not array");P();}
void t7(){T(json_has_function_name);auto j=RustAsyncIntentLowering::toJson(RustAsyncIntentLowering::lower(src));C(j[0].contains("functionName"),"functionName missing");P();}
void t8(){T(json_has_await_count);auto j=RustAsyncIntentLowering::toJson(RustAsyncIntentLowering::lower(src));C(j[0].contains("awaitCount"),"awaitCount missing");P();}
void t9(){T(json_has_returns_future);auto j=RustAsyncIntentLowering::toJson(RustAsyncIntentLowering::lower(src));C(j[0].contains("returnsFuture"),"returnsFuture missing");P();}
void t10(){T(deterministic);auto a=RustAsyncIntentLowering::toJson(RustAsyncIntentLowering::lower(src)).dump();auto b=RustAsyncIntentLowering::toJson(RustAsyncIntentLowering::lower(src)).dump();C(a==b,"nondeterministic");P();}

int main(){std::cout<<"Step 704: Async intent lowering\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
