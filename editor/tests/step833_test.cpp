// Step 833: StableCorpusCertifier tests (10 tests)
#include "graduation/StableCorpusCertifier.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(empty_cases_not_certified);
    auto r=StableCorpusCertifier::certify("py->cpp",{});
    C(!r.certified,"not certified");P();}

void t2(){T(single_pass);
    CorpusCase c{"C1","py->cpp","src","expected",false};
    auto r=StableCorpusCertifier::certify("py->cpp",{c});
    C(r.cases[0].passed,"passed");P();}

void t3(){T(single_fail_empty_snippet);
    CorpusCase c{"C1","py->cpp","","expected",false};
    auto r=StableCorpusCertifier::certify("py->cpp",{c});
    C(!r.cases[0].passed,"not passed");P();}

void t4(){T(partial_pass_not_certified);
    std::vector<CorpusCase> cases={
        {"C1","py->cpp","src","expected",false},
        {"C2","py->cpp","","expected",false}};
    auto r=StableCorpusCertifier::certify("py->cpp",cases);
    C(!r.certified,"partial not certified");P();}

void t5(){T(all_pass_certified);
    std::vector<CorpusCase> cases={
        {"C1","py->cpp","src","expected",false},
        {"C2","py->cpp","src2","expected2",false}};
    auto r=StableCorpusCertifier::certify("py->cpp",cases);
    C(r.certified,"certified");P();}

void t6(){T(total_count);
    std::vector<CorpusCase> cases={
        {"C1","py->cpp","src","exp",false},
        {"C2","py->cpp","src","exp",false}};
    auto r=StableCorpusCertifier::certify("py->cpp",cases);
    C(r.totalCases==2,"totalCases");P();}

void t7(){T(passed_count);
    std::vector<CorpusCase> cases={
        {"C1","py->cpp","src","exp",false},
        {"C2","py->cpp","","exp",false}};
    auto r=StableCorpusCertifier::certify("py->cpp",cases);
    C(r.passedCases==1,"passedCases=1");P();}

void t8(){T(pairId_preserved);
    auto r=StableCorpusCertifier::certify("rust->cpp",{});
    C(r.pairId=="rust->cpp","pairId");P();}

void t9(){T(toJson_output);
    auto r=StableCorpusCertifier::certify("py->cpp",{});
    auto j=StableCorpusCertifier::toJson(r);
    C(j.contains("pair_id"),"pair_id");C(j.contains("certified"),"certified");P();}

void t10(){T(cases_stored_in_result);
    CorpusCase c{"C1","py->cpp","src","exp",false};
    auto r=StableCorpusCertifier::certify("py->cpp",{c});
    C(r.cases.size()==1,"cases size");P();}

int main(){
    std::cout<<"Step 833: StableCorpusCertifier\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
