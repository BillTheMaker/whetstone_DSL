// Step 810: Cross-file API intent inference (10 tests)
#include "legacy_ingestion/CrossFileApiInference.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(read_default);auto pkt=CrossFileApiInference::infer("read",{});C(pkt.inferredIntent=="read","intent");P();}
void t2(){T(write_detection);auto pkt=CrossFileApiInference::infer("writeFile",{"a"});C(pkt.inferredIntent=="write","intent");P();}
void t3(){T(stream_confidence);auto pkt=CrossFileApiInference::infer("streamData",{});C(pkt.confidence>0.8,"conf");P();}
void t4(){T(multi_file_confidence);auto pkt=CrossFileApiInference::infer("api",{"a","b"});C(pkt.confidence>0.6,"conf");P();}
void t5(){T(json_shape);auto j=CrossFileApiInference::toJson(CrossFileApiInference::infer("api",{}));C(j.contains("api")&&j.contains("confidence"),"shape");P();}
void t6(){T(machine_readable);auto j=CrossFileApiInference::toJson(CrossFileApiInference::infer("api",{}));C(j.is_object(),"obj");P();}
void t7(){T(confidence_independent);auto pkt=CrossFileApiInference::infer("read",{});auto pkt2=CrossFileApiInference::infer("read",{});C(pkt.confidence==pkt2.confidence,"conf");P();}
void t8(){T(write_has_higher);auto pkt=CrossFileApiInference::infer("write",{"x","y"});C(pkt.inferredIntent=="write","intent");P();}
void t9(){T(api_label);auto pkt=CrossFileApiInference::infer("api",{});C(!pkt.api.empty(),"api");P();}
void t10(){T(deterministic);auto a=CrossFileApiInference::toJson(CrossFileApiInference::infer("read",{})).dump();auto b=CrossFileApiInference::toJson(CrossFileApiInference::infer("read",{})).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 810: CrossFileApiInference\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
