#include "graduation/WorkspaceModePolicyBindings.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(workspace_id); auto v=WorkspaceModePolicyBindingsFactory::make("ws1","text_first",false); C(v.workspaceId=="ws1","w"); P();}
void t2(){T(default_mode); auto v=WorkspaceModePolicyBindingsFactory::make("ws1","text_first",false); C(v.defaultMode=="text_first","m"); P();}
void t3(){T(locked); auto v=WorkspaceModePolicyBindingsFactory::make("ws1","text_first",true); C(v.locked,"l"); P();}
void t4(){T(valid_text); auto v=WorkspaceModePolicyBindingsFactory::make("ws1","text_first",false); C(v.valid,"v"); P();}
void t5(){T(valid_ast); auto v=WorkspaceModePolicyBindingsFactory::make("ws1","ast_first",false); C(v.valid,"v"); P();}
void t6(){T(valid_hybrid); auto v=WorkspaceModePolicyBindingsFactory::make("ws1","hybrid",false); C(v.valid,"v"); P();}
void t7(){T(invalid_empty_workspace); auto v=WorkspaceModePolicyBindingsFactory::make("","text_first",false); C(!v.valid,"v"); P();}
void t8(){T(invalid_mode); auto v=WorkspaceModePolicyBindingsFactory::make("ws1","unknown",false); C(!v.valid,"v"); P();}
void t9(){T(json_locked); auto j=WorkspaceModePolicyBindingsFactory::toJson(WorkspaceModePolicyBindingsFactory::make("ws1","text_first",true)); C(j["locked"].get<bool>(),"j"); P();}
void t10(){T(deterministic); auto a=WorkspaceModePolicyBindingsFactory::toJson(WorkspaceModePolicyBindingsFactory::make("ws1","hybrid",true)); auto b=WorkspaceModePolicyBindingsFactory::toJson(WorkspaceModePolicyBindingsFactory::make("ws1","hybrid",true)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1650: Workspace mode policy bindings\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
