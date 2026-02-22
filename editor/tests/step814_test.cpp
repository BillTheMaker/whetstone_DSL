// Step 814: Confidence calibration (10 tests)
#include "legacy_ingestion/ConfidenceCalibration.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(high_confidence);C(ConfidenceCalibration::calibrate(0.9)==0.95,"c");P();}
void t2(){T(medium_confidence);C(ConfidenceCalibration::calibrate(0.6)>0.6,"c");P();}
void t3(){T(low_confidence);C(ConfidenceCalibration::calibrate(0.4)==0.4,"c");P();}
void t4(){T(json_shape);auto j=ConfidenceCalibration::toJson(ConfidenceCalibration::calibrate(0.7));C(j.contains("calibrated_confidence"),"shape");P();}
void t5(){T(machine_readable);auto j=ConfidenceCalibration::toJson(ConfidenceCalibration::calibrate(0.2));C(j.is_object(),"obj");P();}
void t6(){T(deterministic);auto a=ConfidenceCalibration::toJson(ConfidenceCalibration::calibrate(0.3)).dump();auto b=ConfidenceCalibration::toJson(ConfidenceCalibration::calibrate(0.3)).dump();C(a==b,"det");P();}
void t7(){T(calibrate_zero);C(ConfidenceCalibration::calibrate(0.0)==0.0,"c");P();}
void t8(){T(calibrate_boundary);C(ConfidenceCalibration::calibrate(0.8)==0.9,"c");P();}
void t9(){T(calibrate_high);C(ConfidenceCalibration::calibrate(1.0)==0.95,"c");P();}
void t10(){T(calibrate_low);C(ConfidenceCalibration::calibrate(0.5)==0.5,"c");P();}
int main(){std::cout<<"Step 814: ConfidenceCalibration\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
