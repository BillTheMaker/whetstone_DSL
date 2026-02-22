// Step 840: PairSelectionStrategyEngine tests (10 tests)
#include "graduation/PairSelectionStrategy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(full_sweep);
    std::vector<std::string> pairs={"a","b","c","d","e"};
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::FullSweep,pairs,3);
    C(r.selectedPairs.size()==5,"all pairs");P();}

void t2(){T(hot_pairs_count);
    std::vector<std::string> pairs={"a","b","c","d","e"};
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::HotPairs,pairs,3);
    C(r.selectedPairs.size()==3,"3 pairs");P();}

void t3(){T(random_sample_count);
    std::vector<std::string> pairs={"a","b","c","d","e"};
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::RandomSample,pairs,3);
    C(r.selectedPairs.size()==3,"3 pairs");P();}

void t4(){T(empty_pairs);
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::FullSweep,{},3);
    C(r.selectedPairs.empty(),"empty");P();}

void t5(){T(full_sweep_rationale);
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::FullSweep,{"a"},3);
    C(r.rationale=="full_sweep","rationale");P();}

void t6(){T(hot_pairs_rationale);
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::HotPairs,{"a"},3);
    C(r.rationale=="hot_pairs","rationale");P();}

void t7(){T(sample_size_respected);
    std::vector<std::string> pairs={"a","b","c","d","e"};
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::HotPairs,pairs,2);
    C(r.selectedPairs.size()==2,"size=2");P();}

void t8(){T(all_pairs_in_full_sweep);
    std::vector<std::string> pairs={"x","y","z"};
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::FullSweep,pairs,1);
    C(r.selectedPairs.size()==3,"all 3");P();}

void t9(){T(sample_size_gt_available);
    std::vector<std::string> pairs={"a","b"};
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::HotPairs,pairs,10);
    C(r.selectedPairs.size()==2,"capped at available");P();}

void t10(){T(random_sample_rationale);
    auto r=PairSelectionStrategyEngine::select(SelectionStrategy::RandomSample,{"a"},3);
    C(r.rationale=="random_sample","rationale");P();}

int main(){
    std::cout<<"Step 840: PairSelectionStrategyEngine\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
