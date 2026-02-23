// Step 862: Hint confidence + uncertainty packet format (10 tests)
#include "graduation/HintConfidencePacket.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(hint_id_set);
    auto p2=HintConfidencePacketModel::make("H-1",0.9f,0.1f,"model");
    C(p2.hintId=="H-1","id");P();}
void t2(){T(confidence_stored);
    auto p2=HintConfidencePacketModel::make("H-1",0.75f,0.1f,"model");
    C(p2.confidence==0.75f,"conf");P();}
void t3(){T(band_high);
    auto p2=HintConfidencePacketModel::make("H-1",0.9f,0.1f,"m");
    C(p2.confidenceBand=="high","high");P();}
void t4(){T(band_medium);
    auto p2=HintConfidencePacketModel::make("H-1",0.65f,0.1f,"m");
    C(p2.confidenceBand=="medium","medium");P();}
void t5(){T(band_low);
    auto p2=HintConfidencePacketModel::make("H-1",0.3f,0.1f,"m");
    C(p2.confidenceBand=="low","low");P();}
void t6(){T(band_unknown);
    auto p2=HintConfidencePacketModel::make("H-1",0.1f,0.1f,"m");
    C(p2.confidenceBand=="unknown","unknown");P();}
void t7(){T(escalate_high_uncertainty);
    auto p2=HintConfidencePacketModel::make("H-1",0.9f,0.6f,"m");
    C(p2.shouldEscalate,"escalate");P();}
void t8(){T(no_escalate_normal);
    auto p2=HintConfidencePacketModel::make("H-1",0.9f,0.1f,"m");
    C(!p2.shouldEscalate,"no esc");P();}
void t9(){T(escalate_low_confidence);
    auto p2=HintConfidencePacketModel::make("H-1",0.2f,0.1f,"m");
    C(p2.shouldEscalate,"esc low conf");P();}
void t10(){T(to_json_has_band);
    auto p2=HintConfidencePacketModel::make("H-1",0.9f,0.1f,"m");
    auto j=HintConfidencePacketModel::toJson(p2);
    C(j.contains("band"),"band");P();}

int main(){
    std::cout<<"Step 862: Hint confidence + uncertainty packet\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
