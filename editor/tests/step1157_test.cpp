#include "graduation/InteropPublicationBundle.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(id); auto b=InteropPublicationBundleFactory::make("b1","m1","h1","r1",true); C(b.bundleId=="b1","i"); P();}
void t2(){T(manifest); auto b=InteropPublicationBundleFactory::make("b1","m1","h1","r1",true); C(b.manifestId=="m1","m"); P();}
void t3(){T(harness); auto b=InteropPublicationBundleFactory::make("b1","m1","h1","r1",true); C(b.referenceHarnessId=="h1","h"); P();}
void t4(){T(report); auto b=InteropPublicationBundleFactory::make("b1","m1","h1","r1",true); C(b.conformanceReportId=="r1","r"); P();}
void t5(){T(signed_true); auto b=InteropPublicationBundleFactory::make("b1","m1","h1","r1",true); C(b.signedBundle,"s"); P();}
void t6(){T(status_publishable); auto b=InteropPublicationBundleFactory::make("b1","m1","h1","r1",true); C(b.status=="publishable","p"); P();}
void t7(){T(status_draft); auto b=InteropPublicationBundleFactory::make("b1","m1","h1","r1",false); C(b.status=="draft","d"); P();}
void t8(){T(json_status); auto j=InteropPublicationBundleFactory::toJson(InteropPublicationBundleFactory::make("b1","m1","h1","r1",true)); C(j["status"]=="publishable","j"); P();}
void t9(){T(json_bundle); auto j=InteropPublicationBundleFactory::toJson(InteropPublicationBundleFactory::make("b1","m1","h1","r1",true)); C(j["bundle_id"]=="b1","j"); P();}
void t10(){T(deterministic); auto a=InteropPublicationBundleFactory::toJson(InteropPublicationBundleFactory::make("b1","m1","h1","r1",true)); auto b=InteropPublicationBundleFactory::toJson(InteropPublicationBundleFactory::make("b1","m1","h1","r1",true)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1157: Interop publication bundle\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
