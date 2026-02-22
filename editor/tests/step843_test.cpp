// Step 843: BlastRadiusAnalyzer tests (10 tests)
#include "graduation/BlastRadiusAnalyzer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(no_entries_no_risk);
    auto r=BlastRadiusAnalyzer::analyze("F1",{});
    C(r.riskLevel=="none","none risk");P();}

void t2(){T(single_pair_low);
    BlastRadiusEntry e{"semantics",{"py->cpp"},10};
    auto r=BlastRadiusAnalyzer::analyze("F1",{e});
    C(r.riskLevel=="low","low");P();}

void t3(){T(many_pairs_high);
    BlastRadiusEntry e{"semantics",{"a","b","c","d","e","f","g","h","i","j"},10};
    auto r=BlastRadiusAnalyzer::analyze("F1",{e});
    C(r.riskLevel=="high","high");P();}

void t4(){T(total_count);
    BlastRadiusEntry e{"semantics",{"a","b"},10};
    auto r=BlastRadiusAnalyzer::analyze("F1",{e});
    C(r.totalAffectedPairs==2,"total=2");P();}

void t5(){T(riskLevel_none);
    auto r=BlastRadiusAnalyzer::analyze("F1",{});
    C(r.riskLevel=="none","none");P();}

void t6(){T(riskLevel_medium);
    BlastRadiusEntry e{"sem",{"a","b","c","d"},5};
    auto r=BlastRadiusAnalyzer::analyze("F1",{e});
    C(r.riskLevel=="medium","medium");P();}

void t7(){T(triggerFeature_preserved);
    auto r=BlastRadiusAnalyzer::analyze("MY_FEATURE",{});
    C(r.triggerFeature=="MY_FEATURE","trigger");P();}

void t8(){T(entries_stored);
    BlastRadiusEntry e{"sem",{"a"},5};
    auto r=BlastRadiusAnalyzer::analyze("F1",{e});
    C(r.entries.size()==1,"entries");P();}

void t9(){T(toJson);
    auto r=BlastRadiusAnalyzer::analyze("F1",{});
    auto j=BlastRadiusAnalyzer::toJson(r);
    C(j.contains("trigger")&&j.contains("risk_level"),"keys");P();}

void t10(){T(multiple_entries_total);
    std::vector<BlastRadiusEntry> entries={
        {"sem",{"a","b"},5},{"perf",{"c"},3}};
    auto r=BlastRadiusAnalyzer::analyze("F1",entries);
    C(r.totalAffectedPairs==3,"total=3");P();}

int main(){
    std::cout<<"Step 843: BlastRadiusAnalyzer\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
