// Step 850: PacketTaggingIntegration tests (10 tests)
#include "graduation/PacketTaggingIntegration.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(tag_with_codes);
    auto r=PacketTaggingIntegration::tag("PKT1",{"T001","T002"});
    C(r.tagged,"tagged");P();}

void t2(){T(empty_codes_not_tagged);
    auto r=PacketTaggingIntegration::tag("PKT1",{});
    C(!r.tagged,"not tagged");P();}

void t3(){T(packetId_preserved);
    auto r=PacketTaggingIntegration::tag("MY-PKT",{"T001"});
    C(r.packetId=="MY-PKT","packetId");P();}

void t4(){T(tags_stored);
    auto r=PacketTaggingIntegration::tag("PKT1",{"T001","T004"});
    C(r.tags.size()==2,"2 tags");P();}

void t5(){T(toJson_has_keys);
    auto r=PacketTaggingIntegration::tag("PKT1",{"T001"});
    auto j=PacketTaggingIntegration::toJson(r);
    C(j.contains("packet_id")&&j.contains("tags")&&j.contains("tagged"),"keys");P();}

void t6(){T(toJson_tagged_true);
    auto r=PacketTaggingIntegration::tag("PKT1",{"T001"});
    auto j=PacketTaggingIntegration::toJson(r);
    C(j["tagged"]==true,"tagged=true");P();}

void t7(){T(toJson_tagged_false);
    auto r=PacketTaggingIntegration::tag("PKT1",{});
    auto j=PacketTaggingIntegration::toJson(r);
    C(j["tagged"]==false,"tagged=false");P();}

void t8(){T(single_tag);
    auto r=PacketTaggingIntegration::tag("PKT1",{"T003"});
    C(r.tags[0]=="T003","tag T003");P();}

void t9(){T(multiple_packets_independent);
    auto r1=PacketTaggingIntegration::tag("PKT1",{"T001"});
    auto r2=PacketTaggingIntegration::tag("PKT2",{"T004"});
    C(r1.packetId!=r2.packetId,"independent");P();}

void t10(){T(empty_packet_id);
    auto r=PacketTaggingIntegration::tag("",{"T001"});
    C(r.packetId.empty(),"empty packetId");C(r.tagged,"still tagged");P();}

int main(){
    std::cout<<"Step 850: PacketTaggingIntegration\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
