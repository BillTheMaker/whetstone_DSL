// Step 891: Runtime assumption extraction from source projects (10 tests)
#include "graduation/RuntimeAssumptionExtractor.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(empty_patterns_empty_hints);
    auto h=RuntimeAssumptionExtractor::extract("jvm11",{});
    C(h.empty(),"empty");P();}
void t2(){T(one_pattern_one_hint);
    auto h=RuntimeAssumptionExtractor::extract("jvm11",{"import java.util"});
    C(h.size()==1,"one");P();}
void t3(){T(hint_ids_sequential);
    auto h=RuntimeAssumptionExtractor::extract("cpython3",{"p1","p2"});
    C(h[0].hintId=="RH-1"&&h[1].hintId=="RH-2","ids");P();}
void t4(){T(runtime_id_set);
    auto h=RuntimeAssumptionExtractor::extract("nodejs18",{"pat"});
    C(h[0].runtimeId=="nodejs18","id");P();}
void t5(){T(evidence_set);
    auto h=RuntimeAssumptionExtractor::extract("jvm11",{"mypat"});
    C(h[0].evidence=="mypat","evidence");P();}
void t6(){T(confidence_positive);
    auto h=RuntimeAssumptionExtractor::extract("jvm11",{"pat"});
    C(h[0].confidence>0.0f,"conf");P();}
void t7(){T(multiple_patterns);
    auto h=RuntimeAssumptionExtractor::extract("jvm11",{"a","b","c"});
    C(h.size()==3,"three");P();}
void t8(){T(category_set);
    auto h=RuntimeAssumptionExtractor::extract("jvm11",{"pat"});
    C(!h[0].category.empty(),"cat");P();}
void t9(){T(to_json_has_hint_id);
    auto h=RuntimeAssumptionExtractor::extract("jvm11",{"pat"});
    auto j=RuntimeAssumptionExtractor::toJson(h[0]);
    C(j.contains("hint_id"),"json");P();}
void t10(){T(to_json_has_confidence);
    auto h=RuntimeAssumptionExtractor::extract("jvm11",{"pat"});
    auto j=RuntimeAssumptionExtractor::toJson(h[0]);
    C(j.contains("confidence"),"conf");P();}

int main(){
    std::cout<<"Step 891: Runtime assumption extraction\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
