// Step 884: Retry/backoff strategy for unstable pairs (8 tests)
#include "graduation/RetryBackoffStrategy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(first_attempt_should_retry);
    auto d=RetryBackoffStrategy::decide("py->cpp",0,3,100);
    C(d.shouldRetry,"retry");P();}
void t2(){T(max_attempts_no_retry);
    auto d=RetryBackoffStrategy::decide("py->cpp",3,3,100);
    C(!d.shouldRetry,"no retry");P();}
void t3(){T(pair_id_set);
    auto d=RetryBackoffStrategy::decide("rust->go",0,3,100);
    C(d.pairId=="rust->go","pairId");P();}
void t4(){T(attempt_number_stored);
    auto d=RetryBackoffStrategy::decide("py->cpp",2,3,100);
    C(d.attemptNumber==2,"attempt");P();}
void t5(){T(backoff_increases_with_attempt);
    auto d0=RetryBackoffStrategy::decide("py->cpp",0,3,100);
    auto d1=RetryBackoffStrategy::decide("py->cpp",1,3,100);
    C(d1.backoffMs>d0.backoffMs,"backoff");P();}
void t6(){T(no_retry_zero_backoff);
    auto d=RetryBackoffStrategy::decide("py->cpp",3,3,100);
    C(d.backoffMs==0,"zero");P();}
void t7(){T(reason_will_retry);
    auto d=RetryBackoffStrategy::decide("py->cpp",0,3,100);
    C(d.reason=="will_retry","reason");P();}
void t8(){T(reason_max_attempts);
    auto d=RetryBackoffStrategy::decide("py->cpp",3,3,100);
    C(d.reason=="max_attempts_reached","reason");P();}

int main(){
    std::cout<<"Step 884: Retry/backoff strategy\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
