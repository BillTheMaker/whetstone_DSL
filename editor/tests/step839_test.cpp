// Step 839: CertificationJobScheduler tests (12 tests)
#include "graduation/CertificationJobScheduler.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(schedule_success);
    CertificationJobScheduler s;
    CertificationJob j{"J1","py->cpp","now","pending",1};
    C(s.schedule(j),"scheduled");P();}

void t2(){T(duplicate_jobId_fails);
    CertificationJobScheduler s;
    CertificationJob j{"J1","py->cpp","now","pending",1};
    s.schedule(j);
    std::string err;
    C(!s.schedule(j,&err),"dup fail");
    C(err=="job_duplicate","err");P();}

void t3(){T(missing_jobId_fails);
    CertificationJobScheduler s;
    std::string err;
    CertificationJob j{"","py->cpp","now","pending",1};
    C(!s.schedule(j,&err),"missing jobId");
    C(err=="job_id_missing","err");P();}

void t4(){T(missing_pairId_fails);
    CertificationJobScheduler s;
    std::string err;
    CertificationJob j{"J1","","now","pending",1};
    C(!s.schedule(j,&err),"missing pairId");
    C(err=="pair_id_missing","err");P();}

void t5(){T(pending_returns_pending_only);
    CertificationJobScheduler s;
    CertificationJob j1{"J1","py->cpp","now","pending",1};
    CertificationJob j2{"J2","rust->cpp","now","running",1};
    s.schedule(j1);s.schedule(j2);
    auto pend=s.pending();
    C(pend.size()==1,"one pending");P();}

void t6(){T(schedule_multiple);
    CertificationJobScheduler s;
    s.schedule({"J1","a","now","pending",1});
    s.schedule({"J2","b","now","pending",2});
    C(s.total()==2,"two jobs");P();}

void t7(){T(priority_field_stored);
    CertificationJobScheduler s;
    CertificationJob j{"J1","py->cpp","now","pending",5};
    s.schedule(j);
    auto pend=s.pending();
    C(pend[0].priority==5,"priority=5");P();}

void t8(){T(status_pending_by_default);
    CertificationJobScheduler s;
    CertificationJob j{"J1","py->cpp","now","pending",1};
    s.schedule(j);
    auto pend=s.pending();
    C(pend[0].status=="pending","status pending");P();}

void t9(){T(job_count);
    CertificationJobScheduler s;
    s.schedule({"J1","a","now","pending",0});
    s.schedule({"J2","b","now","pending",0});
    s.schedule({"J3","c","now","pending",0});
    C(s.total()==3,"3 jobs");P();}

void t10(){T(order_of_pending);
    CertificationJobScheduler s;
    s.schedule({"J1","a","now","pending",0});
    s.schedule({"J2","b","now","pending",0});
    auto pend=s.pending();
    C(pend[0].jobId=="J1","first");C(pend[1].jobId=="J2","second");P();}

void t11(){T(toJson_output);
    CertificationJob j{"J1","py->cpp","now","pending",1};
    auto jj=CertificationJobScheduler::toJson(j);
    C(jj.contains("job_id")&&jj.contains("status"),"keys");P();}

void t12(){T(no_pending_after_complete);
    CertificationJobScheduler s;
    s.schedule({"J1","a","now","pending",0});
    s.complete("J1",true);
    auto pend=s.pending();
    C(pend.empty(),"no pending");P();}

int main(){
    std::cout<<"Step 839: CertificationJobScheduler\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
