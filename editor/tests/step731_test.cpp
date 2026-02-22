// Step 731: Dependency and supply-chain audit packet (10 tests)
#include "gates/SupplyChainAuditPacket.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static std::vector<DependencyAudit> sample(){return {{"z",1},{"a",0}};}
void t1(){T(build_non_empty);auto packet=SupplyChainAuditPacketModel::build(sample());C(packet.dependencies.size()==2,"size");P();}
void t2(){T(sorted_by_name);auto packet=SupplyChainAuditPacketModel::build(sample());C(packet.dependencies[0].name=="a","sort");P();}
void t3(){T(total_high_sum);auto packet=SupplyChainAuditPacketModel::build(sample());C(packet.totalHigh==1,"sum");P();}
void t4(){T(empty_supported);auto packet=SupplyChainAuditPacketModel::build({});C(packet.dependencies.empty()&&packet.totalHigh==0,"empty");P();}
void t5(){T(multiple_high_sum);auto packet=SupplyChainAuditPacketModel::build({{"a",1},{"b",2}});C(packet.totalHigh==3,"sum");P();}
void t6(){T(json_shape);auto j=SupplyChainAuditPacketModel::toJson(SupplyChainAuditPacketModel::build(sample()));C(j.contains("dependencies")&&j.contains("total_high"),"shape");P();}
void t7(){T(json_dependencies_array);auto j=SupplyChainAuditPacketModel::toJson(SupplyChainAuditPacketModel::build(sample()));C(j["dependencies"].is_array(),"arr");P();}
void t8(){T(json_dep_fields);auto j=SupplyChainAuditPacketModel::toJson(SupplyChainAuditPacketModel::build(sample()));C(j["dependencies"][0].contains("name")&&j["dependencies"][0].contains("high_vulns"),"fields");P();}
void t9(){T(machine_readable);auto j=SupplyChainAuditPacketModel::toJson(SupplyChainAuditPacketModel::build(sample()));C(j.is_object(),"obj");P();}
void t10(){T(deterministic);auto a=SupplyChainAuditPacketModel::toJson(SupplyChainAuditPacketModel::build(sample())).dump();auto b=SupplyChainAuditPacketModel::toJson(SupplyChainAuditPacketModel::build(sample())).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 731: SupplyChainAuditPacket\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
