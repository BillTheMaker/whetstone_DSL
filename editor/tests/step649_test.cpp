// Step 649: AppImage/.deb package generator (12 tests)

#include "PackageScriptGenerator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(rejects_empty_app_name);auto o=PackageScriptGenerator::generate("","1.0.0");C(!o.success,"should fail");P();}
void t2(){T(rejects_empty_version);auto o=PackageScriptGenerator::generate("whetstone","");C(!o.success,"should fail");P();}
void t3(){T(generates_successfully);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.success,"should pass");P();}
void t4(){T(appimage_script_has_shebang);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.appImageScript.find("#!/usr/bin/env bash")!=std::string::npos,"shebang");P();}
void t5(){T(appimage_script_invokes_tool);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.appImageScript.find("appimagetool")!=std::string::npos,"tool missing");P();}
void t6(){T(appimage_script_contains_output_name);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.appImageScript.find("whetstone-1.0.0.AppImage")!=std::string::npos,"name missing");P();}
void t7(){T(debian_control_contains_package);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.debianControl.find("Package: whetstone")!=std::string::npos,"pkg missing");P();}
void t8(){T(debian_control_contains_version);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.debianControl.find("Version: 1.0.0")!=std::string::npos,"version missing");P();}
void t9(){T(debian_control_contains_architecture);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.debianControl.find("Architecture: amd64")!=std::string::npos,"arch missing");P();}
void t10(){T(debian_control_contains_maintainer);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.debianControl.find("Maintainer:")!=std::string::npos,"maintainer missing");P();}
void t11(){T(debian_control_contains_description);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(o.debianControl.find("Description:")!=std::string::npos,"desc missing");P();}
void t12(){T(scripts_non_empty_when_successful);auto o=PackageScriptGenerator::generate("whetstone","1.0.0");C(!o.appImageScript.empty()&&!o.debianControl.empty(),"scripts empty");P();}

int main(){std::cout<<"Step 649: AppImage/.deb package generator\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
