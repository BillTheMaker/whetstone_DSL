// Step 744: Java raising adapter v1 (10 tests)
#include "systems/JavaRaisingAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(target_lang_java);auto r=JavaRaisingAdapterV1::raise("ir","safe");C(r.targetLanguage=="java","lang");P();}
void t2(){T(code_preview_non_empty);auto r=JavaRaisingAdapterV1::raise("ir","safe");C(!r.codePreview.empty(),"code");P();}
void t3(){T(profile_preserved);auto r=JavaRaisingAdapterV1::raise("ir","safe");C(r.profile=="safe","profile");P();}
void t4(){T(code_contains_ir);auto r=JavaRaisingAdapterV1::raise("ir","safe");C(r.codePreview.find("ir")!=std::string::npos,"ir");P();}
void t5(){T(to_json_shape);auto j=JavaRaisingAdapterV1::toJson(JavaRaisingAdapterV1::raise("ir","safe"));C(j.contains("target_language")&&j.contains("code_preview")&&j.contains("profile"),"shape");P();}
void t6(){T(json_target_java);auto j=JavaRaisingAdapterV1::toJson(JavaRaisingAdapterV1::raise("ir","safe"));C(j.value("target_language","")=="java","lang");P();}
void t7(){T(machine_readable);auto j=JavaRaisingAdapterV1::toJson(JavaRaisingAdapterV1::raise("ir","safe"));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=JavaRaisingAdapterV1::toJson(JavaRaisingAdapterV1::raise("ir","safe")).dump();auto b=JavaRaisingAdapterV1::toJson(JavaRaisingAdapterV1::raise("ir","safe")).dump();C(a==b,"det");P();}
void t9(){T(empty_ir_supported);auto r=JavaRaisingAdapterV1::raise("","safe");C(!r.codePreview.empty(),"empty");P();}
void t10(){T(profile_can_change);auto a=JavaRaisingAdapterV1::raise("ir","a");auto b=JavaRaisingAdapterV1::raise("ir","b");C(a.profile!=b.profile,"prof");P();}
int main(){std::cout<<"Step 744: JavaRaisingAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
