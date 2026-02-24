#include "graduation/SessionModeOverrideModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(session_id); auto v=SessionModeOverrideModelFactory::make("s1","text_first",true); C(v.sessionId=="s1","s"); P();}
void t2(){T(requested_mode); auto v=SessionModeOverrideModelFactory::make("s1","text_first",true); C(v.requestedMode=="text_first","m"); P();}
void t3(){T(inherit_workspace_policy); auto v=SessionModeOverrideModelFactory::make("s1","text_first",true); C(v.inheritedWorkspacePolicy,"i"); P();}
void t4(){T(valid_text); auto v=SessionModeOverrideModelFactory::make("s1","text_first",true); C(v.valid,"v"); P();}
void t5(){T(valid_ast); auto v=SessionModeOverrideModelFactory::make("s1","ast_first",true); C(v.valid,"v"); P();}
void t6(){T(valid_hybrid); auto v=SessionModeOverrideModelFactory::make("s1","hybrid",true); C(v.valid,"v"); P();}
void t7(){T(invalid_empty_session); auto v=SessionModeOverrideModelFactory::make("","text_first",true); C(!v.valid,"v"); P();}
void t8(){T(invalid_mode); auto v=SessionModeOverrideModelFactory::make("s1","invalid",true); C(!v.valid,"v"); P();}
void t9(){T(json_requested_mode); auto j=SessionModeOverrideModelFactory::toJson(SessionModeOverrideModelFactory::make("s1","hybrid",false)); C(j["requested_mode"]=="hybrid","j"); P();}
void t10(){T(deterministic); auto a=SessionModeOverrideModelFactory::toJson(SessionModeOverrideModelFactory::make("s1","ast_first",false)); auto b=SessionModeOverrideModelFactory::toJson(SessionModeOverrideModelFactory::make("s1","ast_first",false)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1651: Session-level mode override model\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
