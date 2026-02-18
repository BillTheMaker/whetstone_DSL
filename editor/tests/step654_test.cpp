// Step 654: HiveMind job publisher from editor (12 tests)

#include "HiveMindJobPublisher.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static json task(){return {{"id","job-1"},{"type","refactor"}};}

void t1(){T(rejects_non_object_task);auto r=HiveMindJobPublisher::dispatch(json::array());C(!r.queuedInNexus,"should fail");P();}
void t2(){T(rejects_missing_id);auto r=HiveMindJobPublisher::dispatch({{"type","x"}});C(!r.queuedInNexus,"missing id");P();}
void t3(){T(queues_valid_job_in_nexus);auto r=HiveMindJobPublisher::dispatch(task());C(r.queuedInNexus,"queue");P();}
void t4(){T(publishes_valid_job_to_default_topic);auto r=HiveMindJobPublisher::dispatch(task());C(r.publishedToTopic,"publish");P();}
void t5(){T(default_topic_is_pending_jobs);auto r=HiveMindJobPublisher::dispatch(task());C(r.topic=="borg/jobs/pending","topic");P();}
void t6(){T(custom_topic_is_respected);auto r=HiveMindJobPublisher::dispatch(task(),"borg/jobs/custom");C(r.topic=="borg/jobs/custom","custom topic");P();}
void t7(){T(job_id_is_returned);auto r=HiveMindJobPublisher::dispatch(task());C(r.jobId=="job-1","job id");P();}
void t8(){T(empty_topic_disables_publish_flag);auto r=HiveMindJobPublisher::dispatch(task(),"");C(!r.publishedToTopic,"no publish");P();}
void t9(){T(queued_flag_true_even_with_empty_topic);auto r=HiveMindJobPublisher::dispatch(task(),"");C(r.queuedInNexus,"queued");P();}
void t10(){T(dispatched_job_keeps_topic_string);auto r=HiveMindJobPublisher::dispatch(task(),"x");C(r.topic=="x","topic string");P();}
void t11(){T(id_must_be_non_empty_string);auto r=HiveMindJobPublisher::dispatch({{"id",""}});C(!r.queuedInNexus,"empty id");P();}
void t12(){T(dispatched_job_returns_consistent_flags);auto r=HiveMindJobPublisher::dispatch(task());C(r.queuedInNexus&&r.publishedToTopic,"flags");P();}

int main(){std::cout<<"Step 654: HiveMind job publisher\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
