// Step 774: Macro boundary + hygienic expansion packet model (10 tests)
#include "ast_native/MacroHygieneBoundary.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static ASTNativeLoweringPacket mk(bool macro){ASTNativeLoweringPacket p; p.macroLike=macro; return p;}
void t1(){T(boundary_present_with_macro);auto x=MacroHygieneBoundaryModel::classify(mk(true),"lisp");C(x.macroBoundaryPresent,"boundary");P();}
void t2(){T(boundary_absent_without_macro);auto x=MacroHygieneBoundaryModel::classify(mk(false),"lisp");C(!x.macroBoundaryPresent,"boundary");P();}
void t3(){T(scheme_hygienic_default);auto x=MacroHygieneBoundaryModel::classify(mk(true),"scheme");C(x.hygienicByDefault,"hyg");P();}
void t4(){T(lisp_not_hygienic_default);auto x=MacroHygieneBoundaryModel::classify(mk(true),"lisp");C(!x.hygienicByDefault,"hyg");P();}
void t5(){T(policy_no_macro);auto x=MacroHygieneBoundaryModel::classify(mk(false),"lisp");C(x.expansionPolicy=="no_macro_expansion","policy");P();}
void t6(){T(policy_hygienic);auto x=MacroHygieneBoundaryModel::classify(mk(true),"scheme");C(x.expansionPolicy=="hygienic_expansion","policy");P();}
void t7(){T(policy_guarded);auto x=MacroHygieneBoundaryModel::classify(mk(true),"elisp");C(x.expansionPolicy=="guarded_expansion","policy");P();}
void t8(){T(risk_levels);auto x=MacroHygieneBoundaryModel::classify(mk(true),"elisp");C(x.expansionRisk==2,"risk");P();}
void t9(){T(json_shape);auto j=MacroHygieneBoundaryModel::toJson(MacroHygieneBoundaryModel::classify(mk(true),"scheme"));C(j.contains("expansion_policy")&&j.contains("expansion_risk"),"shape");P();}
void t10(){T(deterministic);auto a=MacroHygieneBoundaryModel::toJson(MacroHygieneBoundaryModel::classify(mk(true),"scheme")).dump();auto b=MacroHygieneBoundaryModel::toJson(MacroHygieneBoundaryModel::classify(mk(true),"scheme")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 774: MacroHygieneBoundary\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
