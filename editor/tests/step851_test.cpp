// Step 851: FailureAggregationIndexer tests (10 tests)
#include "graduation/FailureAggregationIndexer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(index_single_failure);
    FailureAggregationIndexer idx;
    TaggedFailure f2{"F1","py->cpp","T001",{},"msg"};
    idx.index(f2);
    C(idx.totalIndexed()==1,"totalIndexed=1");P();}

void t2(){T(same_code_aggregated);
    FailureAggregationIndexer idx;
    idx.index({"F1","py->cpp","T001",{},"m"});
    idx.index({"F2","py->cpp","T001",{},"m"});
    C(idx.totalIndexed()==2,"totalIndexed=2");P();}

void t3(){T(buckets_count);
    FailureAggregationIndexer idx;
    idx.index({"F1","py->cpp","T001",{},"m"});
    idx.index({"F2","py->cpp","T002",{},"m"});
    C(idx.buckets().size()==2,"2 buckets");P();}

void t4(){T(topBucket_most_frequent);
    FailureAggregationIndexer idx;
    idx.index({"F1","py->cpp","T001",{},"m"});
    idx.index({"F2","py->cpp","T001",{},"m"});
    idx.index({"F3","py->cpp","T002",{},"m"});
    auto top=idx.topBucket();
    C(top.code=="T001","top=T001");P();}

void t5(){T(failureIds_stored_in_bucket);
    FailureAggregationIndexer idx;
    idx.index({"F1","py->cpp","T001",{},"m"});
    auto top=idx.topBucket();
    C(top.failureIds.size()==1,"1 failureId");P();}

void t6(){T(empty_indexer);
    FailureAggregationIndexer idx;
    C(idx.totalIndexed()==0,"empty");P();}

void t7(){T(multiple_different_codes);
    FailureAggregationIndexer idx;
    for(auto& code:{"T001","T002","T003","T004"})
        idx.index({std::string("F")+code,"py->cpp",code,{},"m"});
    C(idx.buckets().size()==4,"4 buckets");P();}

void t8(){T(toJson_output);
    FailureAggregationIndexer idx;
    idx.index({"F1","py->cpp","T001",{},"m"});
    auto j=FailureAggregationIndexer::toJson(idx.buckets());
    C(j.is_array(),"array");P();}

void t9(){T(bucket_count_field);
    FailureAggregationIndexer idx;
    idx.index({"F1","py->cpp","T001",{},"m"});
    idx.index({"F2","py->cpp","T001",{},"m"});
    auto bk=idx.topBucket();
    C(bk.count==2,"count=2");P();}

void t10(){T(index_then_query);
    FailureAggregationIndexer idx;
    idx.index({"F1","py->cpp","T004",{},"m"});
    C(idx.totalIndexed()==1,"indexed");
    C(!idx.buckets().empty(),"buckets");P();}

int main(){
    std::cout<<"Step 851: FailureAggregationIndexer\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
