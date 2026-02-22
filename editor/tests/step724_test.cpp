// Step 724: Fuzz differential runner with seed replay (10 tests)
#include "equiv/FuzzDifferentialRunner.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(non_empty_for_non_empty_seeds);auto v=FuzzDifferentialRunner::run({3,1,2});C(v.size()==3,"size");P();}
void t2(){T(sorted_seeds);auto v=FuzzDifferentialRunner::run({3,1,2});C(v[0].seed==1&&v[2].seed==3,"sort");P();}
void t3(){T(equivalent_true_default);auto v=FuzzDifferentialRunner::run({1});C(v[0].equivalent,"eq");P();}
void t4(){T(empty_supported);auto v=FuzzDifferentialRunner::run({});C(v.empty(),"empty");P();}
void t5(){T(duplicate_seeds_preserved);auto v=FuzzDifferentialRunner::run({2,2});C(v.size()==2,"dup");P();}
void t6(){T(to_json_array);auto j=FuzzDifferentialRunner::toJson(FuzzDifferentialRunner::run({1,2}));C(j.is_array(),"arr");P();}
void t7(){T(json_fields);auto j=FuzzDifferentialRunner::toJson(FuzzDifferentialRunner::run({1}));C(j[0].contains("seed")&&j[0].contains("equivalent"),"fields");P();}
void t8(){T(machine_readable);auto j=FuzzDifferentialRunner::toJson(FuzzDifferentialRunner::run({1}));C(j[0].is_object(),"obj");P();}
void t9(){T(seed_replay_deterministic);auto a=FuzzDifferentialRunner::toJson(FuzzDifferentialRunner::run({9,3,4})).dump();auto b=FuzzDifferentialRunner::toJson(FuzzDifferentialRunner::run({9,3,4})).dump();C(a==b,"det");P();}
void t10(){T(non_negative_seed_supported);auto v=FuzzDifferentialRunner::run({0});C(v[0].seed==0,"zero");P();}

int main(){std::cout<<"Step 724: FuzzDifferentialRunner\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
