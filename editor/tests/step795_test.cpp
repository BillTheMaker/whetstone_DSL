// Step 795: Null/join/aggregation divergence classifier (8 tests)
#include "data_query/QueryDivergenceClassifier.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static QueryLoweringPacket mk(bool j,bool a,bool n){QueryLoweringPacket p; p.hasJoin=j; p.hasAggregate=a; p.hasNullSemantics=n; return p;}
void t1(){T(low_when_no_flags);auto x=QueryDivergenceClassifier::classify(mk(false,false,false));C(x.level=="low","lvl");P();}
void t2(){T(medium_when_one_flag);auto x=QueryDivergenceClassifier::classify(mk(true,false,false));C(x.level=="medium","lvl");P();}
void t3(){T(high_when_two_flags);auto x=QueryDivergenceClassifier::classify(mk(true,true,false));C(x.level=="high","lvl");P();}
void t4(){T(high_when_three_flags);auto x=QueryDivergenceClassifier::classify(mk(true,true,true));C(x.level=="high","lvl");P();}
void t5(){T(reasons_non_empty);auto x=QueryDivergenceClassifier::classify(mk(false,false,false));C(!x.reasons.empty(),"reasons");P();}
void t6(){T(reasons_include_join);auto x=QueryDivergenceClassifier::classify(mk(true,false,false));bool ok=false;for(const auto& r:x.reasons)if(r=="join_semantics")ok=true;C(ok,"reason");P();}
void t7(){T(json_shape);auto j=QueryDivergenceClassifier::toJson(QueryDivergenceClassifier::classify(mk(true,true,false)));C(j.contains("level")&&j.contains("reasons"),"shape");P();}
void t8(){T(deterministic);auto a=QueryDivergenceClassifier::toJson(QueryDivergenceClassifier::classify(mk(true,true,true))).dump();auto b=QueryDivergenceClassifier::toJson(QueryDivergenceClassifier::classify(mk(true,true,true))).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 795: QueryDivergenceClassifier\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
