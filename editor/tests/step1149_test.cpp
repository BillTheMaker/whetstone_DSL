#include "graduation/InteropTestbedManifestModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(id); auto m=InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true); C(m.manifestId=="m1","i"); P();}
void t2(){T(class_name); auto m=InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true); C(m.migrationClass=="core","c"); P();}
void t3(){T(languages); auto m=InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true); C(m.languages.size()==2,"l"); P();}
void t4(){T(test_count); auto m=InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true); C(m.testCaseCount==8,"t"); P();}
void t5(){T(requires_ref); auto m=InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true); C(m.requiresReferenceImpl,"r"); P();}
void t6(){T(valid_true); auto m=InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true); C(m.valid,"v"); P();}
void t7(){T(valid_false); auto m=InteropTestbedManifestModelFactory::make("","core",{"cpp","rust"},8,true); C(!m.valid,"v"); P();}
void t8(){T(json_valid); auto j=InteropTestbedManifestModelFactory::toJson(InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true)); C(j["valid"].get<bool>(),"j"); P();}
void t9(){T(json_count); auto j=InteropTestbedManifestModelFactory::toJson(InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true)); C(j["test_case_count"]==8,"j"); P();}
void t10(){T(deterministic); auto a=InteropTestbedManifestModelFactory::toJson(InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true)); auto b=InteropTestbedManifestModelFactory::toJson(InteropTestbedManifestModelFactory::make("m1","core",{"cpp","rust"},8,true)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1149: Interop testbed manifest model\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
