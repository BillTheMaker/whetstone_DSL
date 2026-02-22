// Step 789: SQL canonical query IR layer (12 tests)
#include "data_query/SqlCanonicalQueryIR.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(dialect_preserved);auto x=SqlCanonicalQueryIR::lower("SELECT 1","postgresql");C(x.sourceDialect=="postgresql","dialect");P();}
void t2(){T(non_empty_ir);auto x=SqlCanonicalQueryIR::lower("SELECT 1","postgresql");C(x.irSummary=="sql_query_ir_v1","ir");P();}
void t3(){T(empty_ir);auto x=SqlCanonicalQueryIR::lower("","postgresql");C(x.irSummary=="empty_query","ir");P();}
void t4(){T(join_detected_upper);auto x=SqlCanonicalQueryIR::lower("SELECT * FROM a JOIN b ON a.id=b.id","postgresql");C(x.hasJoin,"join");P();}
void t5(){T(join_detected_lower);auto x=SqlCanonicalQueryIR::lower("select * from a join b on a.id=b.id","postgresql");C(x.hasJoin,"join");P();}
void t6(){T(aggregate_detected_count);auto x=SqlCanonicalQueryIR::lower("SELECT COUNT(*) FROM a","postgresql");C(x.hasAggregate,"agg");P();}
void t7(){T(aggregate_detected_group_by);auto x=SqlCanonicalQueryIR::lower("SELECT a, SUM(b) FROM t GROUP BY a","postgresql");C(x.hasAggregate,"agg");P();}
void t8(){T(null_detected);auto x=SqlCanonicalQueryIR::lower("SELECT * FROM t WHERE x IS NULL","postgresql");C(x.hasNullSemantics,"null");P();}
void t9(){T(raise_target);auto x=SqlCanonicalQueryIR::raise("sql_query_ir_v1","mysql","safe");C(x.targetDialect=="mysql","target");P();}
void t10(){T(raise_default_profile);auto x=SqlCanonicalQueryIR::raise("sql_query_ir_v1","mysql","");C(x.profile=="safe","profile");P();}
void t11(){T(json_shape);auto j=SqlCanonicalQueryIR::toJson(SqlCanonicalQueryIR::lower("SELECT 1","postgresql"));C(j.contains("source_dialect")&&j.contains("has_join"),"shape");P();}
void t12(){T(deterministic);auto a=SqlCanonicalQueryIR::toJson(SqlCanonicalQueryIR::lower("SELECT COUNT(*) FROM t","postgresql")).dump();auto b=SqlCanonicalQueryIR::toJson(SqlCanonicalQueryIR::lower("SELECT COUNT(*) FROM t","postgresql")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 789: SqlCanonicalQueryIR\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
