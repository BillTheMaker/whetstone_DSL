// Step 864: Hint rollback and suppression controls (8 tests)
#include "graduation/HintRollbackControl.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(suppress_valid);
    HintRollbackControl ctrl;
    HintSuppression s{"S-1","py->cpp","bad_hints",true};
    C(ctrl.suppress(s),"suppress");P();}
void t2(){T(is_suppressed_after_suppress);
    HintRollbackControl ctrl;
    HintSuppression s{"S-1","py->cpp","reason",true};
    ctrl.suppress(s);
    C(ctrl.isSuppressed("py->cpp"),"suppressed");P();}
void t3(){T(not_suppressed_other_pair);
    HintRollbackControl ctrl;
    HintSuppression s{"S-1","py->cpp","reason",true};
    ctrl.suppress(s);
    C(!ctrl.isSuppressed("rust->go"),"not suppressed");P();}
void t4(){T(suppress_empty_pair_fails);
    HintRollbackControl ctrl;
    HintSuppression s{"S-1","","reason",true};
    C(!ctrl.suppress(s),"fail");P();}
void t5(){T(lift_suppression);
    HintRollbackControl ctrl;
    HintSuppression s{"S-1","py->cpp","reason",true};
    ctrl.suppress(s);
    C(ctrl.lift("py->cpp"),"lift");P();}
void t6(){T(not_suppressed_after_lift);
    HintRollbackControl ctrl;
    HintSuppression s{"S-1","py->cpp","reason",true};
    ctrl.suppress(s);
    ctrl.lift("py->cpp");
    C(!ctrl.isSuppressed("py->cpp"),"not suppressed");P();}
void t7(){T(active_count);
    HintRollbackControl ctrl;
    ctrl.suppress({"S-1","py->cpp","r",true});
    ctrl.suppress({"S-2","rust->go","r",true});
    C(ctrl.activeSuppressionsCount()==2,"count");P();}
void t8(){T(to_json_has_suppression_id);
    HintSuppression s{"S-1","py->cpp","reason",true};
    auto j=HintRollbackControl::toJson(s);
    C(j.contains("suppression_id"),"json");P();}

int main(){
    std::cout<<"Step 864: Hint rollback and suppression controls\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
