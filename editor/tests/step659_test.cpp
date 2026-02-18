// Step 659: Entropy scanner - editor side (12 tests)

#include "EntropyScannerModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(scan_counts_duplicate_signatures);auto o=EntropyScannerModel::scan({"a()","a()"},{},{},{});C(o.duplicateSignatures==1,"dup");P();}
void t2(){T(scan_counts_similar_module_names);auto o=EntropyScannerModel::scan({}, {"modAlpha","modBeta"},{},{});C(o.similarModuleNames>=1,"similar");P();}
void t3(){T(scan_counts_unused_exports);auto o=EntropyScannerModel::scan({}, {},{"x","y"},{"x"});C(o.unusedExports==1,"unused");P();}
void t4(){T(score_sums_components);EntropyObservation o{1,2,3};C(EntropyScannerModel::entropyScore(o)==6,"score");P();}
void t5(){T(suggest_refactor_true_at_threshold);EntropyObservation o{1,1,1};C(EntropyScannerModel::shouldSuggestRefactor(o,3),"suggest");P();}
void t6(){T(suggest_refactor_false_below_threshold);EntropyObservation o{1,0,0};C(!EntropyScannerModel::shouldSuggestRefactor(o,3),"no suggest");P();}
void t7(){T(empty_inputs_yield_zeroes);auto o=EntropyScannerModel::scan({}, {}, {}, {});C(o.duplicateSignatures==0&&o.similarModuleNames==0&&o.unusedExports==0,"zero");P();}
void t8(){T(multiple_duplicate_signatures_counted);auto o=EntropyScannerModel::scan({"a","a","a"},{},{},{});C(o.duplicateSignatures==2,"dups");P();}
void t9(){T(used_export_not_counted_unused);auto o=EntropyScannerModel::scan({}, {}, {"x"}, {"x"});C(o.unusedExports==0,"used");P();}
void t10(){T(threshold_parameter_changes_behavior);EntropyObservation o{1,1,0};C(EntropyScannerModel::shouldSuggestRefactor(o,2)&&!EntropyScannerModel::shouldSuggestRefactor(o,3),"threshold");P();}
void t11(){T(similar_prefix_requires_min_length);auto o=EntropyScannerModel::scan({}, {"ab","ac"},{},{});C(o.similarModuleNames==0,"prefix");P();}
void t12(){T(scan_combined_case);auto o=EntropyScannerModel::scan({"a","a"},{"modA","modB"},{"x"},{});C(EntropyScannerModel::entropyScore(o)>=3,"combined");P();}

int main(){std::cout<<"Step 659: Entropy scanner\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
