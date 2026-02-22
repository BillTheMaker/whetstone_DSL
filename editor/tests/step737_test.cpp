// Step 737: Reporting templates for C++ review teams (8 tests)
#include "gates/CppReviewReportTemplate.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(markdown_non_empty);auto m=CppReviewReportTemplate::renderMarkdown("demo",1,false,12.5);C(!m.empty(),"md");P();}
void t2(){T(markdown_contains_project);auto m=CppReviewReportTemplate::renderMarkdown("demo",1,false,12.5);C(m.find("demo")!=std::string::npos,"proj");P();}
void t3(){T(markdown_contains_findings);auto m=CppReviewReportTemplate::renderMarkdown("demo",2,false,12.5);C(m.find("2")!=std::string::npos,"find");P();}
void t4(){T(markdown_contains_sanitizer);auto m=CppReviewReportTemplate::renderMarkdown("demo",1,true,12.5);C(m.find("yes")!=std::string::npos,"san");P();}
void t5(){T(markdown_contains_perf);auto m=CppReviewReportTemplate::renderMarkdown("demo",1,false,12.5);C(m.find("12.500000")!=std::string::npos,"perf");P();}
void t6(){T(to_json_shape);auto j=CppReviewReportTemplate::toJson(CppReviewReportTemplate::renderMarkdown("demo",1,false,12.5));C(j.contains("markdown"),"shape");P();}
void t7(){T(machine_readable);auto j=CppReviewReportTemplate::toJson(CppReviewReportTemplate::renderMarkdown("demo",1,false,12.5));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=CppReviewReportTemplate::toJson(CppReviewReportTemplate::renderMarkdown("demo",1,false,12.5)).dump();auto b=CppReviewReportTemplate::toJson(CppReviewReportTemplate::renderMarkdown("demo",1,false,12.5)).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 737: CppReviewReportTemplate\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
