// Step 831: KnownLimitationsGenerator tests (10 tests)
#include "graduation/KnownLimitationsGenerator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(empty_bundle);
    auto b=KnownLimitationsGenerator::generate("python->cpp",{});
    C(b.limitations.empty(),"empty");C(b.highCount==0,"high=0");P();}

void t2(){T(single_high_severity);
    KnownLimitation lim{"L1","python->cpp","semantics","desc","workaround","high"};
    auto b=KnownLimitationsGenerator::generate("python->cpp",{lim});
    C(b.highCount==1,"highCount");P();}

void t3(){T(counts_across_severities);
    std::vector<KnownLimitation> items={
        {"L1","py->cpp","semantics","d","w","high"},
        {"L2","py->cpp","perf","d","w","medium"},
        {"L3","py->cpp","syntax","d","w","low"}};
    auto b=KnownLimitationsGenerator::generate("py->cpp",items);
    C(b.highCount==1&&b.mediumCount==1&&b.lowCount==1,"counts");P();}

void t4(){T(pairId_preserved);
    auto b=KnownLimitationsGenerator::generate("rust->cpp",{});
    C(b.pairId=="rust->cpp","pairId");P();}

void t5(){T(toJson_output);
    KnownLimitation lim{"L1","py->cpp","semantics","desc","workaround","high"};
    auto b=KnownLimitationsGenerator::generate("py->cpp",{lim});
    auto j=KnownLimitationsGenerator::toJson(b);
    C(j.contains("pair_id"),"pair_id");C(j.contains("limitations"),"limitations");P();}

void t6(){T(limitations_stored);
    KnownLimitation lim{"L1","py->cpp","semantics","desc","w","high"};
    auto b=KnownLimitationsGenerator::generate("py->cpp",{lim});
    C(b.limitations.size()==1,"size");P();}

void t7(){T(medium_count);
    std::vector<KnownLimitation> items={
        {"L1","py->cpp","perf","d","w","medium"},
        {"L2","py->cpp","perf","d","w","medium"}};
    auto b=KnownLimitationsGenerator::generate("py->cpp",items);
    C(b.mediumCount==2,"medium=2");P();}

void t8(){T(low_count);
    KnownLimitation lim{"L1","py->cpp","syntax","d","w","low"};
    auto b=KnownLimitationsGenerator::generate("py->cpp",{lim});
    C(b.lowCount==1,"low=1");P();}

void t9(){T(toJson_has_high_key);
    auto b=KnownLimitationsGenerator::generate("py->cpp",{});
    auto j=KnownLimitationsGenerator::toJson(b);
    C(j.contains("high"),"high key");P();}

void t10(){T(limitation_fields);
    KnownLimitation lim{"L1","py->cpp","semantics","description","workaround","high"};
    auto b=KnownLimitationsGenerator::generate("py->cpp",{lim});
    C(b.limitations[0].limitationId=="L1","limitationId");
    C(b.limitations[0].severity=="high","severity");P();}

int main(){
    std::cout<<"Step 831: KnownLimitationsGenerator\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
