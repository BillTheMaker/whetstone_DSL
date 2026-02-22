// Step 791: T-SQL lowering/raising adapters (10 tests)
#include "data_query/TSqlAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_tsql);auto x=TSqlAdapterV1::lower("SELECT 1");C(x.sourceDialect=="tsql","dialect");P();}
void t2(){T(non_empty_ir);auto x=TSqlAdapterV1::lower("SELECT 1");C(x.irSummary=="sql_query_ir_v1","ir");P();}
void t3(){T(join_detected);auto x=TSqlAdapterV1::lower("SELECT * FROM a JOIN b ON a.id=b.id");C(x.hasJoin,"join");P();}
void t4(){T(aggregate_detected);auto x=TSqlAdapterV1::lower("SELECT COUNT(*) FROM a");C(x.hasAggregate,"agg");P();}
void t5(){T(null_detected);auto x=TSqlAdapterV1::lower("SELECT * FROM a WHERE x IS NULL");C(x.hasNullSemantics,"null");P();}
void t6(){T(raise_target);auto x=TSqlAdapterV1::raise("sql_query_ir_v1","safe");C(x.targetDialect=="tsql","target");P();}
void t7(){T(raise_profile);auto x=TSqlAdapterV1::raise("sql_query_ir_v1","safe");C(x.profile=="safe","profile");P();}
void t8(){T(json_lower_shape);auto j=SqlCanonicalQueryIR::toJson(TSqlAdapterV1::lower("SELECT 1"));C(j.contains("source_dialect")&&j.contains("ir_summary"),"shape");P();}
void t9(){T(json_raise_shape);auto j=SqlCanonicalQueryIR::toJson(TSqlAdapterV1::raise("sql_query_ir_v1","safe"));C(j.contains("target_dialect")&&j.contains("query_preview"),"shape");P();}
void t10(){T(deterministic);auto a=SqlCanonicalQueryIR::toJson(TSqlAdapterV1::lower("SELECT 1")).dump();auto b=SqlCanonicalQueryIR::toJson(TSqlAdapterV1::lower("SELECT 1")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 791: TSqlAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
