#include "graduation/CppTextFirstDefaultProfile.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(profile_id); auto v=CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","text_first",true); C(v.profileId=="cpp-default","p"); P();}
void t2(){T(language_cpp); auto v=CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","text_first",true); C(v.language=="cpp","l"); P();}
void t3(){T(default_mode); auto v=CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","text_first",true); C(v.defaultMode=="text_first","m"); P();}
void t4(){T(deterministic_projection); auto v=CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","text_first",true); C(v.deterministicProjection,"d"); P();}
void t5(){T(valid_true); auto v=CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","text_first",true); C(v.valid,"v"); P();}
void t6(){T(invalid_language); auto v=CppTextFirstDefaultProfileFactory::make("cpp-default","rust","text_first",true); C(!v.valid,"v"); P();}
void t7(){T(invalid_mode); auto v=CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","hybrid",true); C(!v.valid,"v"); P();}
void t8(){T(invalid_profile); auto v=CppTextFirstDefaultProfileFactory::make("","cpp","text_first",true); C(!v.valid,"v"); P();}
void t9(){T(json_defaults); auto j=CppTextFirstDefaultProfileFactory::toJson(CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","text_first",true)); C(j["default_mode"]=="text_first","j"); P();}
void t10(){T(deterministic_json); auto a=CppTextFirstDefaultProfileFactory::toJson(CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","text_first",false)); auto b=CppTextFirstDefaultProfileFactory::toJson(CppTextFirstDefaultProfileFactory::make("cpp-default","cpp","text_first",false)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1652: C++ text-first default profile model\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
