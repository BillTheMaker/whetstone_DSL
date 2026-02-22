// Step 794: Query behavior equivalence runner (10 tests)
#include "data_query/QueryBehaviorEquivalenceRunner.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(equivalent_same_rows);auto x=QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",1}}}));C(x.equivalent,"eq");P();}
void t2(){T(not_equivalent_diff_rows);auto x=QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",2}}}));C(!x.equivalent,"neq");P();}
void t3(){T(diff_rows_count_nonzero);auto x=QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",2}}}));C(x.differingRows>=1,"diff");P();}
void t4(){T(diff_rows_size_delta);auto x=QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}},{{"id",2}}}),nlohmann::json::array({{{"id",1}}}));C(x.differingRows==1,"diff");P();}
void t5(){T(equivalent_diffrows_zero);auto x=QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",1}}}));C(x.differingRows==0,"diff");P();}
void t6(){T(json_shape);auto j=QueryBehaviorEquivalenceRunner::toJson(QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",1}}})));C(j.contains("equivalent")&&j.contains("differing_rows"),"shape");P();}
void t7(){T(machine_readable);auto j=QueryBehaviorEquivalenceRunner::toJson(QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",1}}})));C(j.is_object(),"obj");P();}
void t8(){T(deterministic_equal);auto a=QueryBehaviorEquivalenceRunner::toJson(QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",1}}}))).dump();auto b=QueryBehaviorEquivalenceRunner::toJson(QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",1}}}))).dump();C(a==b,"det");P();}
void t9(){T(deterministic_diff);auto a=QueryBehaviorEquivalenceRunner::toJson(QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",2}}}))).dump();auto b=QueryBehaviorEquivalenceRunner::toJson(QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}),nlohmann::json::array({{{"id",2}}}))).dump();C(a==b,"det");P();}
void t10(){T(non_array_inputs_handled);auto x=QueryBehaviorEquivalenceRunner::compare(nlohmann::json::object(),nlohmann::json::object());C(x.equivalent,"eq");P();}
int main(){std::cout<<"Step 794: QueryBehaviorEquivalenceRunner\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
