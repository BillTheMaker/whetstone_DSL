// Step 767: Family promotion checks (8 tests)
#include "managed/ManagedFamilyPromotion.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(default_pairs_non_empty);auto v=ManagedFamilyPromotionMatrix::defaultPairs();C(!v.empty(),"pairs");P();}
void t2(){T(lookup_known_beta);auto t=ManagedFamilyPromotionMatrix::lookupTier("kotlin","csharp");C(t=="beta","tier");P();}
void t3(){T(lookup_known_experimental);auto t=ManagedFamilyPromotionMatrix::lookupTier("vbnet","kotlin");C(t=="experimental","tier");P();}
void t4(){T(lookup_unknown_defaults_experimental);auto t=ManagedFamilyPromotionMatrix::lookupTier("kotlin","swift");C(t=="experimental","tier");P();}
void t5(){T(report_counts);auto r=ManagedFamilyPromotionMatrix::evaluate(ManagedFamilyPromotionMatrix::defaultPairs());C(r.pairCount==12,"count");C(r.betaCount>=1,"beta");P();}
void t6(){T(report_json_shape);auto j=ManagedFamilyPromotionMatrix::toJson(ManagedFamilyPromotionMatrix::evaluate(ManagedFamilyPromotionMatrix::defaultPairs()));C(j.contains("pair_count")&&j.contains("pairs"),"shape");P();}
void t7(){T(sorted_order);auto s=ManagedFamilyPromotionMatrix::sorted(ManagedFamilyPromotionMatrix::defaultPairs());C(s.front().source<=s.back().source,"sorted");P();}
void t8(){T(deterministic);auto a=ManagedFamilyPromotionMatrix::toJson(ManagedFamilyPromotionMatrix::evaluate(ManagedFamilyPromotionMatrix::defaultPairs())).dump();auto b=ManagedFamilyPromotionMatrix::toJson(ManagedFamilyPromotionMatrix::evaluate(ManagedFamilyPromotionMatrix::defaultPairs())).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 767: ManagedFamilyPromotion\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
