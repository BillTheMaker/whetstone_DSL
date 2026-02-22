// Step 716: build artifact generator (8 tests)

#include "cpp_ir/CppBuildArtifactGenerator.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(cmake_non_empty);auto a=CppBuildArtifactGenerator::generate("demo");C(!a.cmakeLists.empty(),"cmake");P();}
void t2(){T(cmake_has_project);auto a=CppBuildArtifactGenerator::generate("demo");C(a.cmakeLists.find("project(demo")!=std::string::npos,"project");P();}
void t3(){T(cmake_has_target);auto a=CppBuildArtifactGenerator::generate("demo");C(a.cmakeLists.find("add_library(demo")!=std::string::npos,"target");P();}
void t4(){T(headers_present);auto a=CppBuildArtifactGenerator::generate("demo");C(!a.headers.empty(),"headers");P();}
void t5(){T(sources_present);auto a=CppBuildArtifactGenerator::generate("demo");C(!a.sources.empty(),"sources");P();}
void t6(){T(safe_first_define_present);auto a=CppBuildArtifactGenerator::generate("demo","safe-first");C(a.cmakeLists.find("CPP_SAFE_FIRST")!=std::string::npos,"define");P();}
void t7(){T(json_object);auto j=CppBuildArtifactGenerator::toJson(CppBuildArtifactGenerator::generate("demo"));C(j.is_object(),"json");P();}
void t8(){T(deterministic);auto a=CppBuildArtifactGenerator::toJson(CppBuildArtifactGenerator::generate("demo")).dump();auto b=CppBuildArtifactGenerator::toJson(CppBuildArtifactGenerator::generate("demo")).dump();C(a==b,"nondeterministic");P();}

int main(){std::cout<<"Step 716: build artifact generator\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
