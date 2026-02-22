// Step 765: ADT + pattern matching canonical lowering (8 tests)
#include "managed/ADTPatternCanonicalLowering.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(adt_detected_sealed);auto x=ADTPatternCanonicalLowering::lower("sealed class S");C(x.adtDetected,"adt");P();}
void t2(){T(adt_detected_record);auto x=ADTPatternCanonicalLowering::lower("record R(int x)");C(x.adtDetected,"adt");P();}
void t3(){T(pattern_detected_when);auto x=ADTPatternCanonicalLowering::lower("when(x){}");C(x.patternMatchDetected,"pat");P();}
void t4(){T(pattern_detected_match);auto x=ADTPatternCanonicalLowering::lower("match x with");C(x.patternMatchDetected,"pat");P();}
void t5(){T(combined_shape);auto x=ADTPatternCanonicalLowering::lower("sealed class S\nwhen(x){}");C(x.canonicalShape=="sum_type_with_patterns","shape");P();}
void t6(){T(adt_only_shape);auto x=ADTPatternCanonicalLowering::lower("record R(int x)");C(x.canonicalShape=="sum_type","shape");P();}
void t7(){T(nominal_shape);auto x=ADTPatternCanonicalLowering::lower("class C{}");C(x.canonicalShape=="nominal","shape");P();}
void t8(){T(deterministic);auto a=ADTPatternCanonicalLowering::toJson(ADTPatternCanonicalLowering::lower("sealed class S\nwhen(x){}")).dump();auto b=ADTPatternCanonicalLowering::toJson(ADTPatternCanonicalLowering::lower("sealed class S\nwhen(x){}")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 765: ADTPatternCanonicalLowering\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
