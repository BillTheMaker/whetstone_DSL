// Step 706: Unsafe block risk packet generator (8 tests)

#include "rust_ir/RustUnsafeRiskPacket.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static const char* src = "unsafe { let p: *const i32 = core::ptr::null(); }\nextern \"C\" { fn f(); }";

void t1(){T(unsafe_block_count);auto r=RustUnsafeRiskPacketGenerator::analyze(src);C(r.unsafeBlockCount==1,"unsafe count");P();}
void t2(){T(raw_pointer_count);auto r=RustUnsafeRiskPacketGenerator::analyze(src);C(r.rawPointerOps>=1,"raw pointer count");P();}
void t3(){T(ffi_count);auto r=RustUnsafeRiskPacketGenerator::analyze(src);C(r.ffiCalls==1,"ffi count");P();}
void t4(){T(review_required_true);auto r=RustUnsafeRiskPacketGenerator::analyze(src);C(r.reviewRequired,"review required false");P();}
void t5(){T(confidence_lowered);auto r=RustUnsafeRiskPacketGenerator::analyze(src);C(r.confidence<0.9f,"confidence not lowered");P();}
void t6(){T(reasons_present);auto r=RustUnsafeRiskPacketGenerator::analyze(src);C(!r.reasons.empty(),"reasons missing");P();}
void t7(){T(json_has_fields);auto j=RustUnsafeRiskPacketGenerator::toJson(RustUnsafeRiskPacketGenerator::analyze(src));C(j.contains("reviewRequired")&&j.contains("confidence"),"fields missing");P();}
void t8(){T(deterministic);auto a=RustUnsafeRiskPacketGenerator::toJson(RustUnsafeRiskPacketGenerator::analyze(src)).dump();auto b=RustUnsafeRiskPacketGenerator::toJson(RustUnsafeRiskPacketGenerator::analyze(src)).dump();C(a==b,"nondeterministic");P();}

int main(){std::cout<<"Step 706: Unsafe risk packet\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
