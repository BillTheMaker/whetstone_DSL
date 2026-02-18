// Step 660: whetstone_generate_inference_job MCP tool (12 tests)

#include "InferenceJobGenerator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(tool_name_matches_spec);C(InferenceJobGenerator::toolName()=="whetstone_generate_inference_job","tool");P();}
void t2(){T(rejects_empty_goal);auto j=InferenceJobGenerator::generate("",1,{"a.cpp"});C(!j.value("success",true),"should fail");P();}
void t3(){T(rejects_negative_entropy);auto j=InferenceJobGenerator::generate("goal",-1,{"a.cpp"});C(!j.value("success",true),"should fail");P();}
void t4(){T(generates_success_for_valid_input);auto j=InferenceJobGenerator::generate("goal",3,{"a.cpp"});C(j.value("success",false),"success");P();}
void t5(){T(job_type_is_refactor);auto j=InferenceJobGenerator::generate("goal",3,{"a.cpp"});C(j["job"].value("type","")=="refactor","type");P();}
void t6(){T(goal_is_preserved);auto j=InferenceJobGenerator::generate("extract common logic",3,{"a.cpp"});C(j["job"].value("goal","")=="extract common logic","goal");P();}
void t7(){T(entropy_score_is_in_context);auto j=InferenceJobGenerator::generate("goal",7,{"a.cpp"});C(j["job"]["context"].value("entropy_score",-1)==7,"score");P();}
void t8(){T(files_are_included_in_context);auto j=InferenceJobGenerator::generate("goal",3,{"a.cpp","b.cpp"});C(j["job"]["context"]["files"].size()==2,"files");P();}
void t9(){T(default_bounty_is_normal);auto j=InferenceJobGenerator::generate("goal",3,{"a.cpp"});C(j["job"].value("bounty","")=="normal","bounty");P();}
void t10(){T(custom_bounty_is_respected);auto j=InferenceJobGenerator::generate("goal",3,{"a.cpp"},"high");C(j["job"].value("bounty","")=="high","bounty");P();}
void t11(){T(empty_files_allowed);auto j=InferenceJobGenerator::generate("goal",1,{});C(j.value("success",false),"empty files allowed");P();}
void t12(){T(error_code_present_on_invalid_input);auto j=InferenceJobGenerator::generate("",1,{"a.cpp"});C(j.value("error","")=="invalid_input","error");P();}

int main(){std::cout<<"Step 660: Inference job generator\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
