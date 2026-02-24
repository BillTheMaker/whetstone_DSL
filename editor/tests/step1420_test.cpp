#include "graduation/JobOutputCanonicalizationAndTruncationUtilities.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(id); auto v=JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true); C(v.id=="x1","i"); P();}
void t2(){T(detail); auto v=JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true); C(v.detail=="d1","d"); P();}
void t3(){T(score); auto v=JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true); C(v.score==7,"s"); P();}
void t4(){T(enabled); auto v=JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true); C(v.enabled,"e"); P();}
void t5(){T(valid_true); auto v=JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true); C(v.valid,"v"); P();}
void t6(){T(valid_false); auto v=JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("","d1",7,true); C(!v.valid,"v"); P();}
void t7(){T(json_valid); auto j=JobOutputCanonicalizationAndTruncationUtilitiesFactory::toJson(JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true)); C(j["valid"].get<bool>(),"j"); P();}
void t8(){T(json_score); auto j=JobOutputCanonicalizationAndTruncationUtilitiesFactory::toJson(JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true)); C(j["score"]==7,"j"); P();}
void t9(){T(json_id); auto j=JobOutputCanonicalizationAndTruncationUtilitiesFactory::toJson(JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true)); C(j["id"]=="x1","j"); P();}
void t10(){T(deterministic); auto a=JobOutputCanonicalizationAndTruncationUtilitiesFactory::toJson(JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true)); auto b=JobOutputCanonicalizationAndTruncationUtilitiesFactory::toJson(JobOutputCanonicalizationAndTruncationUtilitiesFactory::make("x1","d1",7,true)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1420: Job-output canonicalization and truncation utilities\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
