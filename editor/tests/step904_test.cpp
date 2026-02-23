// Step 904: Migration audit log (8 tests)
#include "graduation/MigrationAuditLog.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

// Note: makeEntry is an instance method (uses nextId_ state)
void t1(){T(empty_count_zero);
    MigrationAuditLog log;
    C(log.count()==0,"zero");P();}
void t2(){T(append_increments_count);
    MigrationAuditLog log;
    log.append(log.makeEntry("p1","start","desc"));
    C(log.count()==1,"one");P();}
void t3(){T(entry_id_sequential);
    MigrationAuditLog log;
    auto e1=log.makeEntry("p1","start","desc");
    auto e2=log.makeEntry("p1","step","desc");
    C(e1.entryId=="AE-1"&&e2.entryId=="AE-2","ids");P();}
void t4(){T(pair_id_set);
    MigrationAuditLog log;
    auto e=log.makeEntry("myPair","event","detail");
    C(e.pairId=="myPair","id");P();}
void t5(){T(event_set);
    MigrationAuditLog log;
    auto e=log.makeEntry("p1","my_event","detail");
    C(e.event=="my_event","event");P();}
void t6(){T(filter_by_pair);
    MigrationAuditLog log;
    log.append(log.makeEntry("p1","start","d"));
    log.append(log.makeEntry("p2","start","d"));
    auto filtered=log.filterByPair("p1");
    C(filtered.size()==1,"filter");P();}
void t7(){T(clear_removes_all);
    MigrationAuditLog log;
    log.append(log.makeEntry("p1","start","d"));
    log.clear();
    C(log.count()==0,"clear");P();}
void t8(){T(to_json_has_entry_id);
    MigrationAuditLog log;
    auto e=log.makeEntry("p1","start","d");
    auto j=MigrationAuditLog::toJson(e);
    C(j.contains("entry_id"),"json");P();}

int main(){
    std::cout<<"Step 904: Migration audit log\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
