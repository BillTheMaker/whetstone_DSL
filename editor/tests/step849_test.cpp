// Step 849: FailureTaxonomy tests (12 tests)
#include "graduation/FailureTaxonomy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(create_taxonomy_codes);
    auto codes=FailureTaxonomy::allCodes();
    C(codes.size()==8,"8 codes");P();}

void t2(){T(primary_code_assigned);
    auto f2=FailureTaxonomy::tag("F1","py->cpp","T001","msg");
    C(f2.primaryCode=="T001","primaryCode");P();}

void t3(){T(secondary_codes_list);
    auto f2=FailureTaxonomy::tag("F1","py->cpp","T001","msg");
    f2.secondaryCodes.push_back("T002");
    C(f2.secondaryCodes.size()==1,"secondary codes");P();}

void t4(){T(T001_category_semantics);
    auto codes=FailureTaxonomy::allCodes();
    bool found=false;
    for(auto&c:codes) if(c.code=="T001"&&c.category=="semantic") found=true;
    C(found,"T001 semantics");P();}

void t5(){T(T004_category_memory);
    auto codes=FailureTaxonomy::allCodes();
    bool found=false;
    for(auto&c:codes) if(c.code=="T004"&&c.category=="memory") found=true;
    C(found,"T004 memory");P();}

void t6(){T(severity_values);
    auto codes=FailureTaxonomy::allCodes();
    bool hasHigh=false,hasMedium=false,hasLow=false;
    for(auto&c:codes){
        if(c.severity=="high") hasHigh=true;
        if(c.severity=="medium") hasMedium=true;
        if(c.severity=="low") hasLow=true;
    }
    C(hasHigh&&hasMedium&&hasLow,"severities");P();}

void t7(){T(failureId_stored);
    auto f2=FailureTaxonomy::tag("FAIL-1","py->cpp","T001","msg");
    C(f2.failureId=="FAIL-1","failureId");P();}

void t8(){T(pairId_stored);
    auto f2=FailureTaxonomy::tag("F1","py->cpp","T001","msg");
    C(f2.pairId=="py->cpp","pairId");P();}

void t9(){T(toJson_output);
    auto f2=FailureTaxonomy::tag("F1","py->cpp","T001","msg");
    auto j=FailureTaxonomy::toJson(f2);
    C(j.contains("failure_id")&&j.contains("primary_code"),"keys");P();}

void t10(){T(isValid_T001);
    C(FailureTaxonomy::isValid("T001"),"T001 valid");P();}

void t11(){T(isValid_T008);
    C(FailureTaxonomy::isValid("T008"),"T008 valid");P();}

void t12(){T(isValid_invalid_code);
    C(!FailureTaxonomy::isValid("T999"),"T999 invalid");P();}

int main(){
    std::cout<<"Step 849: FailureTaxonomy\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
