// Step 652: Privacy-preserving local telemetry (12 tests)

#include "LocalTelemetryModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(record_session_minutes_adds_value);TelemetryCounters c;LocalTelemetryModel::recordSessionMinutes(&c,30);C(c.sessionMinutes==30,"minutes");P();}
void t2(){T(record_session_minutes_ignores_negative);TelemetryCounters c;LocalTelemetryModel::recordSessionMinutes(&c,-1);C(c.sessionMinutes==0,"negative ignored");P();}
void t3(){T(record_tool_call_increments_counter);TelemetryCounters c;LocalTelemetryModel::recordToolCall(&c);C(c.toolCalls==1,"tool calls");P();}
void t4(){T(record_error_increments_counter);TelemetryCounters c;LocalTelemetryModel::recordError(&c);C(c.errorCount==1,"errors");P();}
void t5(){T(record_generator_use_tracks_name);TelemetryCounters c;LocalTelemetryModel::recordGeneratorUse(&c,"RustGenerator");C(c.generatorUsage["RustGenerator"]==1,"usage");P();}
void t6(){T(record_generator_use_accumulates);TelemetryCounters c;LocalTelemetryModel::recordGeneratorUse(&c,"RustGenerator");LocalTelemetryModel::recordGeneratorUse(&c,"RustGenerator");C(c.generatorUsage["RustGenerator"]==2,"accumulate");P();}
void t7(){T(record_generator_use_ignores_empty_name);TelemetryCounters c;LocalTelemetryModel::recordGeneratorUse(&c,"");C(c.generatorUsage.empty(),"empty ignored");P();}
void t8(){T(weekly_report_contains_minutes);TelemetryCounters c;c.sessionMinutes=42;C(LocalTelemetryModel::weeklyReport(c).find("session_minutes=42")!=std::string::npos,"report minutes");P();}
void t9(){T(weekly_report_contains_tool_calls);TelemetryCounters c;c.toolCalls=9;C(LocalTelemetryModel::weeklyReport(c).find("tool_calls=9")!=std::string::npos,"report tools");P();}
void t10(){T(weekly_report_contains_errors);TelemetryCounters c;c.errorCount=3;C(LocalTelemetryModel::weeklyReport(c).find("errors=3")!=std::string::npos,"report errors");P();}
void t11(){T(null_counter_is_noop_for_recorders);LocalTelemetryModel::recordToolCall(nullptr);LocalTelemetryModel::recordError(nullptr);LocalTelemetryModel::recordSessionMinutes(nullptr,1);P();}
void t12(){T(report_is_local_summary_only);TelemetryCounters c;auto r=LocalTelemetryModel::weeklyReport(c);C(r.find("http")==std::string::npos,"must not include remote endpoints");P();}

int main(){std::cout<<"Step 652: Local telemetry\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
