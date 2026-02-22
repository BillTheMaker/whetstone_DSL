// Step 745: Cross-pair compatibility matrix update (8 tests)
#include "systems/SystemsCompatibilityMatrix.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(default_pairs_non_empty);auto pairs=SystemsCompatibilityMatrix::defaultPairs();C(!pairs.empty(),"pairs");P();}
void t2(){T(required_pair_present);auto pairs=SystemsCompatibilityMatrix::defaultPairs();bool ok=false;for(auto&x:pairs)if(x.source=="c"&&x.target=="cpp")ok=true;C(ok,"pair");P();}
void t3(){T(sorted_order);auto pairs=SystemsCompatibilityMatrix::sorted(SystemsCompatibilityMatrix::defaultPairs());C(pairs.front().source<="c","sort");P();}
void t4(){T(tier_fields_non_empty);auto pairs=SystemsCompatibilityMatrix::defaultPairs();C(!pairs[0].tier.empty(),"tier");P();}
void t5(){T(to_json_array);auto j=SystemsCompatibilityMatrix::toJson(SystemsCompatibilityMatrix::defaultPairs());C(j.is_array(),"arr");P();}
void t6(){T(json_fields);auto j=SystemsCompatibilityMatrix::toJson(SystemsCompatibilityMatrix::defaultPairs());C(j[0].contains("source")&&j[0].contains("target")&&j[0].contains("tier"),"fields");P();}
void t7(){T(machine_readable);auto j=SystemsCompatibilityMatrix::toJson(SystemsCompatibilityMatrix::defaultPairs());C(j[0].is_object(),"obj");P();}
void t8(){T(deterministic);auto a=SystemsCompatibilityMatrix::toJson(SystemsCompatibilityMatrix::defaultPairs()).dump();auto b=SystemsCompatibilityMatrix::toJson(SystemsCompatibilityMatrix::defaultPairs()).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 745: SystemsCompatibilityMatrix\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
