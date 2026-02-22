// Step 834: RegressionWatchlist tests (10 tests)
#include "graduation/RegressionWatchlist.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(add_success);
    RegressionWatchlist wl;
    WatchlistEntry e{"py->cpp","risk","now",true};
    C(wl.add(e),"added");P();}

void t2(){T(duplicate_fails);
    RegressionWatchlist wl;
    WatchlistEntry e{"py->cpp","risk","now",true};
    wl.add(e);
    std::string err;
    C(!wl.add(e,&err),"dup fail");
    C(err=="entry_duplicate","err msg");P();}

void t3(){T(remove_success);
    RegressionWatchlist wl;
    WatchlistEntry e{"py->cpp","risk","now",true};
    wl.add(e);
    C(wl.remove("py->cpp"),"removed");P();}

void t4(){T(remove_non_existent_fails);
    RegressionWatchlist wl;
    std::string err;
    C(!wl.remove("nope",&err),"not found");
    C(err=="entry_not_found","err");P();}

void t5(){T(summarize_counts);
    RegressionWatchlist wl;
    wl.add({"a","r","now",true});
    wl.add({"b","r","now",true});
    auto s=wl.summarize();
    C(s.totalEntries==2,"total");P();}

void t6(){T(active_vs_total);
    RegressionWatchlist wl;
    wl.add({"a","r","now",true});
    wl.add({"b","r","now",true});
    wl.remove("a");
    auto s=wl.summarize();
    C(s.totalEntries==2&&s.activeEntries==1,"active vs total");P();}

void t7(){T(empty_watcher);
    RegressionWatchlist wl;
    auto s=wl.summarize();
    C(s.totalEntries==0,"empty");P();}

void t8(){T(pairId_required);
    RegressionWatchlist wl;
    std::string err;
    WatchlistEntry e{"","risk","now",true};
    C(!wl.add(e,&err),"pairId required");
    C(err=="pair_id_missing","err");P();}

void t9(){T(toJson_has_entries);
    RegressionWatchlist wl;
    auto s=wl.summarize();
    auto j=RegressionWatchlist::toJson(s);
    C(j.contains("entries"),"entries");P();}

void t10(){T(add_multiple_unique);
    RegressionWatchlist wl;
    wl.add({"a","r","now",true});
    wl.add({"b","r","now",true});
    wl.add({"c","r","now",true});
    auto s=wl.summarize();
    C(s.totalEntries==3,"three entries");P();}

int main(){
    std::cout<<"Step 834: RegressionWatchlist\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
