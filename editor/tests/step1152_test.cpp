#include "graduation/CanonicalFixtureAndOracleDatasetPacks.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(id); auto d=CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true); C(d.packId=="p1","i"); P();}
void t2(){T(fixtures); auto d=CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true); C(d.fixtureCount==10,"f"); P();}
void t3(){T(oracles); auto d=CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true); C(d.oracleCount==10,"o"); P();}
void t4(){T(version); auto d=CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true); C(d.version=="2026.02","v"); P();}
void t5(){T(checksummed); auto d=CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true); C(d.checksummed,"c"); P();}
void t6(){T(publishable_true); auto d=CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true); C(d.publishable,"p"); P();}
void t7(){T(publishable_false); auto d=CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",false); C(!d.publishable,"p"); P();}
void t8(){T(json_publishable); auto j=CanonicalFixtureAndOracleDatasetPacksFactory::toJson(CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true)); C(j["publishable"].get<bool>(),"j"); P();}
void t9(){T(json_fixtures); auto j=CanonicalFixtureAndOracleDatasetPacksFactory::toJson(CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true)); C(j["fixture_count"]==10,"j"); P();}
void t10(){T(deterministic); auto a=CanonicalFixtureAndOracleDatasetPacksFactory::toJson(CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true)); auto b=CanonicalFixtureAndOracleDatasetPacksFactory::toJson(CanonicalFixtureAndOracleDatasetPacksFactory::make("p1",10,10,"2026.02",true)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1152: Canonical fixture and oracle dataset packs\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
