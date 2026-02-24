#include "graduation/AuthoringModeStateModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(workspace_id); auto v=AuthoringModeStateModelFactory::make("ws1","text_first",true); C(v.workspaceId=="ws1","w"); P();}
void t2(){T(mode); auto v=AuthoringModeStateModelFactory::make("ws1","text_first",true); C(v.mode=="text_first","m"); P();}
void t3(){T(ast_projection); auto v=AuthoringModeStateModelFactory::make("ws1","text_first",true); C(v.astProjectionEnabled,"a"); P();}
void t4(){T(valid_true_text); auto v=AuthoringModeStateModelFactory::make("ws1","text_first",true); C(v.valid,"v"); P();}
void t5(){T(valid_true_ast); auto v=AuthoringModeStateModelFactory::make("ws1","ast_first",true); C(v.valid,"v"); P();}
void t6(){T(valid_true_hybrid); auto v=AuthoringModeStateModelFactory::make("ws1","hybrid",true); C(v.valid,"v"); P();}
void t7(){T(valid_false_empty_workspace); auto v=AuthoringModeStateModelFactory::make("","text_first",true); C(!v.valid,"v"); P();}
void t8(){T(valid_false_bad_mode); auto v=AuthoringModeStateModelFactory::make("ws1","unknown",true); C(!v.valid,"v"); P();}
void t9(){T(json_mode); auto j=AuthoringModeStateModelFactory::toJson(AuthoringModeStateModelFactory::make("ws1","text_first",true)); C(j["mode"]=="text_first","j"); P();}
void t10(){T(deterministic); auto a=AuthoringModeStateModelFactory::toJson(AuthoringModeStateModelFactory::make("ws1","hybrid",false)); auto b=AuthoringModeStateModelFactory::toJson(AuthoringModeStateModelFactory::make("ws1","hybrid",false)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1649: Authoring mode schema and state model\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
