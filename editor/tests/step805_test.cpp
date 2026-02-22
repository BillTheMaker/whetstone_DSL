// Step 805: Host boundary contract generator (8 tests)
#include "low_level/HostBoundaryContract.h"
#include "low_level/AbiCallingConventionModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(host_false);auto x=HostBoundaryContract::build(AbiCallingConventionModel::describe("","c"));C(!x.hostBoundary,"host");P();}
void t2(){T(host_true);auto x=HostBoundaryContract::build(AbiCallingConventionModel::describe("import","c"));C(x.hostBoundary,"host");P();}
void t3(){T(needs_review_on_host);auto x=HostBoundaryContract::build(AbiCallingConventionModel::describe("import","c"));C(x.needsReview,"review");P();}
void t4(){T(needs_review_on_stdcall);auto x=HostBoundaryContract::build(AbiCallingConventionModel::describe("__stdcall","c"));C(x.needsReview,"review");P();}
void t5(){T(contract_present);auto x=HostBoundaryContract::build(AbiCallingConventionModel::describe("","c"));C(!x.contract.empty(),"contract");P();}
void t6(){T(json_shape);auto j=HostBoundaryContract::toJson(HostBoundaryContract::build(AbiCallingConventionModel::describe("","c")));C(j.contains("contract")&&j.contains("needs_review"),"shape");P();}
void t7(){T(machine_readable);auto j=HostBoundaryContract::toJson(HostBoundaryContract::build(AbiCallingConventionModel::describe("","c")));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=HostBoundaryContract::toJson(HostBoundaryContract::build(AbiCallingConventionModel::describe("","c")));auto b=HostBoundaryContract::toJson(HostBoundaryContract::build(AbiCallingConventionModel::describe("","c")));C(a==b,"det");P();}
int main(){std::cout<<"Step 805: HostBoundaryContract\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
