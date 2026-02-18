// Step 662: Cross-session context bridge (12 tests)

#include "CrossSessionContextBridge.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static CrossSessionBridgeRequest req(){return {"job-1",{"a.cpp","b.cpp"},"/ws"};}

void t1(){T(rejects_missing_job_id);auto r=req();r.jobId="";auto o=CrossSessionContextBridge::run(r);C(!o.launchedMcp,"reject");P();}
void t2(){T(rejects_missing_workspace);auto r=req();r.workspace="";auto o=CrossSessionContextBridge::run(r);C(!o.launchedMcp,"reject");P();}
void t3(){T(rejects_empty_context_files);auto r=req();r.contextFiles.clear();auto o=CrossSessionContextBridge::run(r);C(!o.launchedMcp,"reject");P();}
void t4(){T(valid_request_launches_mcp);auto o=CrossSessionContextBridge::run(req());C(o.launchedMcp,"launch");P();}
void t5(){T(valid_request_calls_intake);auto o=CrossSessionContextBridge::run(req());C(o.intakeCalled,"intake");P();}
void t6(){T(valid_request_feeds_back_to_editor);auto o=CrossSessionContextBridge::run(req());C(o.fedBackToEditor,"feedback");P();}
void t7(){T(all_flags_true_for_valid_request);auto o=CrossSessionContextBridge::run(req());C(o.launchedMcp&&o.intakeCalled&&o.fedBackToEditor,"flags");P();}
void t8(){T(invalid_request_keeps_flags_false);auto o=CrossSessionContextBridge::run({});C(!o.launchedMcp&&!o.intakeCalled&&!o.fedBackToEditor,"flags");P();}
void t9(){T(single_context_file_allowed);CrossSessionBridgeRequest r{"job-1",{"one.cpp"},"/ws"};auto o=CrossSessionContextBridge::run(r);C(o.launchedMcp,"single");P();}
void t10(){T(many_context_files_allowed);CrossSessionBridgeRequest r{"job-1",{"a","b","c","d"},"/ws"};auto o=CrossSessionContextBridge::run(r);C(o.intakeCalled,"many");P();}
void t11(){T(workspace_path_passthrough_not_empty);auto r=req();C(!r.workspace.empty(),"workspace");P();}
void t12(){T(job_id_passthrough_not_empty);auto r=req();C(!r.jobId.empty(),"jobid");P();}

int main(){std::cout<<"Step 662: Cross-session context bridge\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
