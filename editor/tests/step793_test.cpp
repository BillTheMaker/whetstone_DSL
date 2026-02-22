// Step 793: Transaction/isolation semantics packet model (10 tests)
#include "data_query/TransactionIsolationPacket.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(transaction_detected_begin);auto x=TransactionIsolationModel::analyze("BEGIN TRANSACTION","read_committed");C(x.transactionDetected,"tx");P();}
void t2(){T(transaction_detected_commit);auto x=TransactionIsolationModel::analyze("COMMIT","read_committed");C(x.transactionDetected,"tx");P();}
void t3(){T(isolation_serializable);auto x=TransactionIsolationModel::analyze("SET TRANSACTION ISOLATION LEVEL SERIALIZABLE","read_committed");C(x.isolationLevel=="serializable","iso");P();}
void t4(){T(isolation_repeatable);auto x=TransactionIsolationModel::analyze("SET TRANSACTION ISOLATION LEVEL REPEATABLE READ","read_committed");C(x.isolationLevel=="repeatable_read","iso");P();}
void t5(){T(isolation_default_read_committed);auto x=TransactionIsolationModel::analyze("SELECT 1","read_committed");C(x.isolationLevel=="read_committed","iso");P();}
void t6(){T(risk_on_downgrade_true);auto x=TransactionIsolationModel::analyze("SERIALIZABLE","read_committed");C(x.riskOnDowngrade,"risk");P();}
void t7(){T(risk_on_downgrade_false);auto x=TransactionIsolationModel::analyze("SERIALIZABLE","serializable");C(!x.riskOnDowngrade,"risk");P();}
void t8(){T(json_shape);auto j=TransactionIsolationModel::toJson(TransactionIsolationModel::analyze("BEGIN","read_committed"));C(j.contains("transaction_detected")&&j.contains("isolation_level"),"shape");P();}
void t9(){T(machine_readable);auto j=TransactionIsolationModel::toJson(TransactionIsolationModel::analyze("BEGIN","read_committed"));C(j.is_object(),"obj");P();}
void t10(){T(deterministic);auto a=TransactionIsolationModel::toJson(TransactionIsolationModel::analyze("SERIALIZABLE","read_committed")).dump();auto b=TransactionIsolationModel::toJson(TransactionIsolationModel::analyze("SERIALIZABLE","read_committed")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 793: TransactionIsolationModel\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
