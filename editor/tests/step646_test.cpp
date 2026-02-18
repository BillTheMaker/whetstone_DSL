// Step 646: In-editor test runner (12 tests)

#include "InEditorTestRunnerModel.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(run_marks_complete);auto s=InEditorTestRunnerModel::run({"step1_test"});C(s.complete,"not complete");P();}
void t2(){T(run_counts_passes);auto s=InEditorTestRunnerModel::run({"step1_test","step2_test"});C(s.passed==2,"pass count");P();}
void t3(){T(run_counts_failures);auto s=InEditorTestRunnerModel::run({"step_fail_test"});C(s.failed==1,"fail count");P();}
void t4(){T(run_creates_item_per_target);auto s=InEditorTestRunnerModel::run({"a","b"});C(s.items.size()==2,"item count");P();}
void t5(){T(pass_output_marker);auto s=InEditorTestRunnerModel::run({"a"});C(s.items[0].output=="PASS","pass marker");P();}
void t6(){T(fail_output_marker);auto s=InEditorTestRunnerModel::run({"fail_target"});C(s.items[0].output=="FAIL","fail marker");P();}
void t7(){T(failure_link_format);C(InEditorTestRunnerModel::linkForFailure("step646_test")=="editor/tests/step646_test.cpp","link mismatch");P();}
void t8(){T(empty_run_is_complete);auto s=InEditorTestRunnerModel::run({});C(s.complete,"empty not complete");P();}
void t9(){T(empty_run_has_zero_counts);auto s=InEditorTestRunnerModel::run({});C(s.passed==0&&s.failed==0,"counts not zero");P();}
void t10(){T(mixed_run_counts_both);auto s=InEditorTestRunnerModel::run({"a","fail_b"});C(s.passed==1&&s.failed==1,"mixed counts");P();}
void t11(){T(item_target_preserved);auto s=InEditorTestRunnerModel::run({"step646_test"});C(s.items[0].target=="step646_test","target mismatch");P();}
void t12(){T(failure_detection_uses_name_pattern);auto s=InEditorTestRunnerModel::run({"my_fail_case"});C(!s.items[0].passed,"should fail");P();}

int main(){std::cout<<"Step 646: In-editor test runner\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
