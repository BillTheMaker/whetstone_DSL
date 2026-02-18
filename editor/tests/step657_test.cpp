// Step 657: Energy context status bar (12 tests)

#include "EnergyContextStatusModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(label_for_high_solar_uses_watts);EnergyContextState e{"HighSolar",450,80};C(EnergyContextStatusModel::statusLabel(e)=="HighSolar 450W","label");P();}
void t2(){T(label_for_conservation_uses_percent);EnergyContextState e{"Conservation",120,28};C(EnergyContextStatusModel::statusLabel(e)=="Conservation 28%","label");P();}
void t3(){T(flash_true_for_conservation);EnergyContextState e{"Conservation",120,50};C(EnergyContextStatusModel::shouldFlashOrange(e),"flash");P();}
void t4(){T(flash_true_for_low_battery);EnergyContextState e{"Normal",120,20};C(EnergyContextStatusModel::shouldFlashOrange(e),"flash");P();}
void t5(){T(flash_false_for_normal_high_battery);EnergyContextState e{"Normal",120,80};C(!EnergyContextStatusModel::shouldFlashOrange(e),"no flash");P();}
void t6(){T(prompt_context_line_prefix);EnergyContextState e{"Normal",100,70};C(EnergyContextStatusModel::promptContextLine(e).find("energyState:")==0,"prefix");P();}
void t7(){T(prompt_context_includes_label);EnergyContextState e{"HighSolar",450,80};C(EnergyContextStatusModel::promptContextLine(e).find("HighSolar 450W")!=std::string::npos,"content");P();}
void t8(){T(default_state_label);EnergyContextState e;C(EnergyContextStatusModel::statusLabel(e)=="Normal 0W","default");P();}
void t9(){T(conservation_low_battery_still_percent_label);EnergyContextState e{"Conservation",10,5};C(EnergyContextStatusModel::statusLabel(e)=="Conservation 5%","label");P();}
void t10(){T(battery_threshold_30_not_flashing);EnergyContextState e{"Normal",100,30};C(!EnergyContextStatusModel::shouldFlashOrange(e),"threshold");P();}
void t11(){T(battery_threshold_29_flashing);EnergyContextState e{"Normal",100,29};C(EnergyContextStatusModel::shouldFlashOrange(e),"threshold");P();}
void t12(){T(prompt_context_for_conservation);EnergyContextState e{"Conservation",120,28};C(EnergyContextStatusModel::promptContextLine(e).find("Conservation 28%")!=std::string::npos,"prompt");P();}

int main(){std::cout<<"Step 657: Energy context status model\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
