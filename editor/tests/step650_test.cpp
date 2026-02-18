// Step 650: HiveMind-push auto-update mechanism (12 tests)

#include "HiveMindAutoUpdateModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static UpdateJob job(){return {"1.2.3","https://apiary/whetstone.AppImage","abc123"};}

void t1(){T(valid_job_true);C(HiveMindAutoUpdateModel::isValidJob(job()),"valid expected");P();}
void t2(){T(invalid_job_missing_version);auto j=job();j.version="";C(!HiveMindAutoUpdateModel::isValidJob(j),"invalid expected");P();}
void t3(){T(invalid_job_missing_url);auto j=job();j.downloadUrl="";C(!HiveMindAutoUpdateModel::isValidJob(j),"invalid expected");P();}
void t4(){T(invalid_job_missing_hash);auto j=job();j.expectedSha256="";C(!HiveMindAutoUpdateModel::isValidJob(j),"invalid expected");P();}
void t5(){T(hash_verify_true_for_match);C(HiveMindAutoUpdateModel::verifyHash("abc123","abc123"),"hash match");P();}
void t6(){T(hash_verify_false_for_mismatch);C(!HiveMindAutoUpdateModel::verifyHash("abc124","abc123"),"hash mismatch");P();}
void t7(){T(execute_rejects_invalid_job);auto j=job();j.version="";auto r=HiveMindAutoUpdateModel::execute(j,"abc123");C(!r.accepted,"should reject");P();}
void t8(){T(execute_accepts_valid_job);auto r=HiveMindAutoUpdateModel::execute(job(),"abc123");C(r.accepted,"should accept");P();}
void t9(){T(execute_sets_hash_verified_on_match);auto r=HiveMindAutoUpdateModel::execute(job(),"abc123");C(r.hashVerified,"hash verified");P();}
void t10(){T(execute_blocks_replace_on_hash_mismatch);auto r=HiveMindAutoUpdateModel::execute(job(),"zzz");C(!r.binaryReplaced,"replace blocked");P();}
void t11(){T(execute_replaces_binary_on_verified_hash);auto r=HiveMindAutoUpdateModel::execute(job(),"abc123");C(r.binaryReplaced,"replace expected");P();}
void t12(){T(execute_schedules_restart_on_success);auto r=HiveMindAutoUpdateModel::execute(job(),"abc123");C(r.restartScheduled,"restart expected");P();}

int main(){std::cout<<"Step 650: HiveMind auto-update mechanism\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
