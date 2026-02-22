// Step 743: Go raising adapter v1 (10 tests)
#include "systems/GoRaisingAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(target_lang_go);auto r=GoRaisingAdapterV1::raise("ir","safe");C(r.targetLanguage=="go","lang");P();}
void t2(){T(code_preview_non_empty);auto r=GoRaisingAdapterV1::raise("ir","safe");C(!r.codePreview.empty(),"code");P();}
void t3(){T(profile_preserved);auto r=GoRaisingAdapterV1::raise("ir","safe");C(r.profile=="safe","profile");P();}
void t4(){T(code_contains_ir);auto r=GoRaisingAdapterV1::raise("ir","safe");C(r.codePreview.find("ir")!=std::string::npos,"ir");P();}
void t5(){T(to_json_shape);auto j=GoRaisingAdapterV1::toJson(GoRaisingAdapterV1::raise("ir","safe"));C(j.contains("target_language")&&j.contains("code_preview")&&j.contains("profile"),"shape");P();}
void t6(){T(json_target_go);auto j=GoRaisingAdapterV1::toJson(GoRaisingAdapterV1::raise("ir","safe"));C(j.value("target_language","")=="go","lang");P();}
void t7(){T(machine_readable);auto j=GoRaisingAdapterV1::toJson(GoRaisingAdapterV1::raise("ir","safe"));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=GoRaisingAdapterV1::toJson(GoRaisingAdapterV1::raise("ir","safe")).dump();auto b=GoRaisingAdapterV1::toJson(GoRaisingAdapterV1::raise("ir","safe")).dump();C(a==b,"det");P();}
void t9(){T(empty_ir_supported);auto r=GoRaisingAdapterV1::raise("","safe");C(!r.codePreview.empty(),"empty");P();}
void t10(){T(profile_can_change);auto a=GoRaisingAdapterV1::raise("ir","a");auto b=GoRaisingAdapterV1::raise("ir","b");C(a.profile!=b.profile,"prof");P();}
int main(){std::cout<<"Step 743: GoRaisingAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
