// Step 721: Rust runner adapter for harness (10 tests)
#include "equiv/RustRunnerAdapter.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static std::vector<TestVector> sample(){return {{"v1",{{"x",1}},"a"},{"v2",{{"x",2}},"b"}};}

void t1(){T(runner_name);auto r=RustRunnerAdapter::run(sample());C(r.runner=="rust","name");P();}
void t2(){T(output_count_matches_vectors);auto r=RustRunnerAdapter::run(sample());C(r.outputs.size()==2,"count");P();}
void t3(){T(string_expected_output);auto r=RustRunnerAdapter::run(sample());C(r.outputs[0]=="a","out");P();}
void t4(){T(json_expected_output);std::vector<TestVector> v={{"v",{{"x",1}},nlohmann::json{{"ok",true}}}};auto r=RustRunnerAdapter::run(v);C(r.outputs[0].find("ok")!=std::string::npos,"json");P();}
void t5(){T(empty_vectors_supported);auto r=RustRunnerAdapter::run({});C(r.outputs.empty(),"empty");P();}
void t6(){T(to_json_shape);auto j=RustRunnerAdapter::toJson(RustRunnerAdapter::run(sample()));C(j.contains("runner")&&j.contains("outputs"),"shape");P();}
void t7(){T(outputs_array);auto j=RustRunnerAdapter::toJson(RustRunnerAdapter::run(sample()));C(j["outputs"].is_array(),"arr");P();}
void t8(){T(machine_readable);auto j=RustRunnerAdapter::toJson(RustRunnerAdapter::run(sample()));C(j.is_object(),"obj");P();}
void t9(){T(replay_deterministic);auto a=RustRunnerAdapter::toJson(RustRunnerAdapter::run(sample())).dump();auto b=RustRunnerAdapter::toJson(RustRunnerAdapter::run(sample())).dump();C(a==b,"det");P();}
void t10(){T(non_empty_when_input_non_empty);auto r=RustRunnerAdapter::run(sample());C(!r.outputs.empty(),"nonempty");P();}

int main(){std::cout<<"Step 721: RustRunnerAdapter\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
