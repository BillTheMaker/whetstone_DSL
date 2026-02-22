// Step 847: CertificationDashboard tests (8 tests)
#include "graduation/CertificationDashboard.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(empty_state);
    auto s=CertificationDashboard::build({});
    C(s.certified==0&&s.failed==0&&s.pending==0,"empty");P();}

void t2(){T(single_certified);
    DashboardEntry e{"py->cpp","C1","certified",95,"stable"};
    auto s=CertificationDashboard::build({e});
    C(s.certified==1,"certified=1");P();}

void t3(){T(counts);
    std::vector<DashboardEntry> entries={
        {"a","C1","certified",90,"stable"},
        {"b","C1","failed",50,"beta"},
        {"c","C1","pending",0,"experimental"}};
    auto s=CertificationDashboard::build(entries);
    C(s.certified==1&&s.failed==1&&s.pending==1,"counts");P();}

void t4(){T(toJson);
    auto s=CertificationDashboard::build({});
    auto j=CertificationDashboard::toJson(s);
    C(j.contains("certified")&&j.contains("entries"),"keys");P();}

void t5(){T(mixed_statuses);
    std::vector<DashboardEntry> entries={
        {"a","C1","certified",90,"stable"},
        {"b","C1","certified",85,"stable"}};
    auto s=CertificationDashboard::build(entries);
    C(s.certified==2,"certified=2");P();}

void t6(){T(entries_preserved);
    DashboardEntry e{"py->cpp","C1","certified",95,"stable"};
    auto s=CertificationDashboard::build({e});
    C(s.entries.size()==1,"entries size");P();}

void t7(){T(failed_count);
    std::vector<DashboardEntry> entries={
        {"a","C1","failed",30,"beta"},
        {"b","C1","failed",40,"beta"}};
    auto s=CertificationDashboard::build(entries);
    C(s.failed==2,"failed=2");P();}

void t8(){T(pending_count);
    DashboardEntry e{"c","C1","pending",0,"experimental"};
    auto s=CertificationDashboard::build({e});
    C(s.pending==1,"pending=1");P();}

int main(){
    std::cout<<"Step 847: CertificationDashboard\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
