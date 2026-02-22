// Step 725: Behavioral divergence packet + minimization (8 tests)
#include "equiv/BehavioralDivergencePacket.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(equivalent_classification);auto packet=BehavioralDivergencePacketModel::classify("c","x","x","input");C(packet.classification=="equivalent","class");P();}
void t2(){T(divergence_classification);auto packet=BehavioralDivergencePacketModel::classify("c","x","y","input");C(packet.classification=="unacceptable_divergence","class");P();}
void t3(){T(minimize_short_input_kept);auto packet=BehavioralDivergencePacketModel::classify("c","x","y","short");C(packet.minimizedInput=="short","min");P();}
void t4(){T(minimize_long_input_cut);auto packet=BehavioralDivergencePacketModel::classify("c","x","y","0123456789abcdefXYZ");C(packet.minimizedInput.size()==16,"min");P();}
void t5(){T(case_id_preserved);auto packet=BehavioralDivergencePacketModel::classify("cid","x","y","in");C(packet.caseId=="cid","id");P();}
void t6(){T(json_shape);auto j=BehavioralDivergencePacketModel::toJson(BehavioralDivergencePacketModel::classify("c","x","y","in"));C(j.contains("case_id")&&j.contains("classification")&&j.contains("minimized_input"),"shape");P();}
void t7(){T(machine_readable);auto j=BehavioralDivergencePacketModel::toJson(BehavioralDivergencePacketModel::classify("c","x","y","in"));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=BehavioralDivergencePacketModel::toJson(BehavioralDivergencePacketModel::classify("c","x","y","input_value")).dump();auto b=BehavioralDivergencePacketModel::toJson(BehavioralDivergencePacketModel::classify("c","x","y","input_value")).dump();C(a==b,"det");P();}

int main(){std::cout<<"Step 725: BehavioralDivergencePacket\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
