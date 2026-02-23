// Step 900: Migration path ranker (8 tests)
#include "graduation/MigrationPathRanker.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(rank_empty_returns_empty);
    auto r=MigrationPathRanker::rank({});
    C(r.empty(),"empty");P();}
void t2(){T(rank_single);
    auto c=MigrationPathCandidateFactory::make("p1","src","tgt",{"a"},0.7f,"ok");
    auto r=MigrationPathRanker::rank({c});
    C(r.size()==1,"one");P();}
void t3(){T(rank_descending_feasibility);
    auto c1=MigrationPathCandidateFactory::make("p1","src","tgt",{"a"},0.3f,"ok");
    auto c2=MigrationPathCandidateFactory::make("p2","src","tgt",{"a"},0.9f,"ok");
    auto r=MigrationPathRanker::rank({c1,c2});
    C(r[0].feasibility>r[1].feasibility,"desc");P();}
void t4(){T(rank_preserves_count);
    auto c1=MigrationPathCandidateFactory::make("p1","src","tgt",{"a"},0.5f,"ok");
    auto c2=MigrationPathCandidateFactory::make("p2","src","tgt",{"a"},0.7f,"ok");
    auto c3=MigrationPathCandidateFactory::make("p3","src","tgt",{"a"},0.3f,"ok");
    auto r=MigrationPathRanker::rank({c1,c2,c3});
    C(r.size()==3,"count");P();}
void t5(){T(top_n_limits);
    auto c1=MigrationPathCandidateFactory::make("p1","src","tgt",{"a"},0.5f,"ok");
    auto c2=MigrationPathCandidateFactory::make("p2","src","tgt",{"a"},0.7f,"ok");
    auto c3=MigrationPathCandidateFactory::make("p3","src","tgt",{"a"},0.9f,"ok");
    auto r=MigrationPathRanker::topN({c1,c2,c3},2);
    C(r.size()==2,"top2");P();}
void t6(){T(top_n_returns_best);
    auto c1=MigrationPathCandidateFactory::make("p1","src","tgt",{"a"},0.1f,"ok");
    auto c2=MigrationPathCandidateFactory::make("p2","src","tgt",{"a"},0.9f,"ok");
    auto r=MigrationPathRanker::topN({c1,c2},1);
    C(r[0].feasibility==0.9f,"best");P();}
void t7(){T(top_n_larger_than_count);
    auto c1=MigrationPathCandidateFactory::make("p1","src","tgt",{"a"},0.5f,"ok");
    auto r=MigrationPathRanker::topN({c1},5);
    C(r.size()==1,"bounded");P();}
void t8(){T(rank_first_has_highest_feasibility);
    auto c1=MigrationPathCandidateFactory::make("p1","src","tgt",{"a"},0.4f,"ok");
    auto c2=MigrationPathCandidateFactory::make("p2","src","tgt",{"a"},0.8f,"ok");
    auto r=MigrationPathRanker::rank({c1,c2});
    C(r[0].pairId=="p2","first");P();}

int main(){
    std::cout<<"Step 900: Migration path ranker\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
