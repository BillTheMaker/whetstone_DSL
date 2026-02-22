// Step 757: Dynamic family acceptance report + review queue wiring (8 tests)
#include "dynamic/DynamicFamilyAcceptanceReport.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static DynamicLoweringPacket dp(const std::string& lang,bool review){DynamicLoweringPacket p; p.sourceLanguage=lang; p.irSummary=lang+"_ir"; p.reviewRequired=review; return p;}
static DynamicRiskPacket rk(const std::string& lvl){DynamicRiskPacket r; r.level=lvl; return r;}
void t1(){T(total_items_count);auto r=DynamicFamilyAcceptanceReportModel::build({dp("python",false),dp("lua",false)},{rk("low"),rk("low")});C(r.totalItems==2,"count");P();}
void t2(){T(review_count_from_packet_flag);auto r=DynamicFamilyAcceptanceReportModel::build({dp("python",true)},{rk("low")});C(r.reviewRequiredCount==1,"review");P();}
void t3(){T(review_count_from_risk);auto r=DynamicFamilyAcceptanceReportModel::build({dp("python",false)},{rk("medium")});C(r.reviewRequiredCount==1,"review");P();}
void t4(){T(review_queue_array);auto r=DynamicFamilyAcceptanceReportModel::build({dp("python",true)},{rk("high")});C(r.reviewQueue.is_array(),"arr");P();}
void t5(){T(review_queue_fields);auto r=DynamicFamilyAcceptanceReportModel::build({dp("python",true)},{rk("high")});C(r.reviewQueue[0].contains("source_language")&&r.reviewQueue[0].contains("ir_summary")&&r.reviewQueue[0].contains("risk_level"),"fields");P();}
void t6(){T(to_json_shape);auto j=DynamicFamilyAcceptanceReportModel::toJson(DynamicFamilyAcceptanceReportModel::build({dp("python",true)},{rk("high")}));C(j.contains("total_items")&&j.contains("review_required_count")&&j.contains("review_queue"),"shape");P();}
void t7(){T(machine_readable);auto j=DynamicFamilyAcceptanceReportModel::toJson(DynamicFamilyAcceptanceReportModel::build({dp("python",true)},{rk("high")}));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=DynamicFamilyAcceptanceReportModel::toJson(DynamicFamilyAcceptanceReportModel::build({dp("python",true)},{rk("high")})).dump();auto b=DynamicFamilyAcceptanceReportModel::toJson(DynamicFamilyAcceptanceReportModel::build({dp("python",true)},{rk("high")})).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 757: DynamicFamilyAcceptanceReport\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
