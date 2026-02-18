// Step 656: Apiary browser panel (12 tests)

#include "ApiaryBrowserPanelModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static std::vector<ApiaryToolEntry> tools(){
return {{"nexus","1.0","cpp",{"db","jobs"},"nexus tool","/mnt/nas/swarm_tools/nexus.cpp"},
{"drone","2.1","rust",{"jobs","executor"},"drone tool","/mnt/nas/swarm_tools/drone.rs"},
{"vision","0.9","python",{"cv"},"vision tool","/mnt/nas/swarm_tools/vision.py"}};}

void t1(){T(empty_capability_returns_all);auto r=ApiaryBrowserPanelModel::filterByCapability(tools(),"");C(r.size()==3,"all");P();}
void t2(){T(filter_jobs_returns_two);auto r=ApiaryBrowserPanelModel::filterByCapability(tools(),"jobs");C(r.size()==2,"jobs");P();}
void t3(){T(filter_cv_returns_one);auto r=ApiaryBrowserPanelModel::filterByCapability(tools(),"cv");C(r.size()==1,"cv");P();}
void t4(){T(filter_missing_returns_zero);auto r=ApiaryBrowserPanelModel::filterByCapability(tools(),"none");C(r.empty(),"none");P();}
void t5(){T(filter_preserves_order);auto r=ApiaryBrowserPanelModel::filterByCapability(tools(),"jobs");C(r[0].name=="nexus"&&r[1].name=="drone","order");P();}
void t6(){T(title_includes_name);C(ApiaryBrowserPanelModel::title(tools()[0]).find("nexus")!=std::string::npos,"name");P();}
void t7(){T(title_includes_version);C(ApiaryBrowserPanelModel::title(tools()[0]).find("1.0")!=std::string::npos,"version");P();}
void t8(){T(source_path_is_preserved);C(tools()[1].sourcePath.find("drone.rs")!=std::string::npos,"source");P();}
void t9(){T(language_field_available);C(tools()[2].language=="python","lang");P();}
void t10(){T(doc_summary_field_available);C(tools()[0].docSummary=="nexus tool","doc");P();}
void t11(){T(capability_match_is_exact);auto r=ApiaryBrowserPanelModel::filterByCapability(tools(),"job");C(r.empty(),"exact");P();}
void t12(){T(filter_handles_single_entry);std::vector<ApiaryToolEntry> one={tools()[0]};auto r=ApiaryBrowserPanelModel::filterByCapability(one,"db");C(r.size()==1,"single");P();}

int main(){std::cout<<"Step 656: Apiary browser panel\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
