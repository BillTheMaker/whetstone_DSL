// Step 720: Deterministic test vector specification format (10 tests)
#include "equiv/TestVectorSpec.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static std::vector<TestVector> sample(){return {{"v2",{{"x",2}},"b"},{"v1",{{"x",1}},"a"}};}

void t1(){T(normalize_sorts);auto v=TestVectorSpec::normalize(sample());C(v[0].id=="v1","sort");P();}
void t2(){T(to_json_array);auto j=TestVectorSpec::toJson(TestVectorSpec::normalize(sample()));C(j.is_array(),"arr");P();}
void t3(){T(to_json_fields);auto j=TestVectorSpec::toJson(TestVectorSpec::normalize(sample()));C(j[0].contains("id")&&j[0].contains("input")&&j[0].contains("expected"),"fields");P();}
void t4(){T(from_json_roundtrip_size);auto v=TestVectorSpec::fromJson(TestVectorSpec::toJson(sample()));C(v.size()==2,"size");P();}
void t5(){T(from_json_sorted);auto v=TestVectorSpec::fromJson(TestVectorSpec::toJson(sample()));C(v[0].id=="v1","sort");P();}
void t6(){T(non_array_returns_empty);auto v=TestVectorSpec::fromJson(nlohmann::json::object());C(v.empty(),"empty");P();}
void t7(){T(input_preserved);auto v=TestVectorSpec::fromJson(TestVectorSpec::toJson(sample()));C(v[0].input.is_object(),"input");P();}
void t8(){T(expected_preserved);auto v=TestVectorSpec::fromJson(TestVectorSpec::toJson(sample()));C(!v[0].expected.is_null(),"exp");P();}
void t9(){T(machine_readable);auto j=TestVectorSpec::toJson(TestVectorSpec::normalize(sample()));C(j[0].is_object(),"obj");P();}
void t10(){T(deterministic);auto a=TestVectorSpec::toJson(TestVectorSpec::normalize(sample())).dump();auto b=TestVectorSpec::toJson(TestVectorSpec::normalize(sample())).dump();C(a==b,"det");P();}

int main(){std::cout<<"Step 720: TestVectorSpec\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
