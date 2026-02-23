// Step 889: Runtime semantics pack schema (12 tests)
#include "graduation/RuntimeSemanticsPack.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(runtime_id_set);
    auto pk=RuntimeSemanticsPackSchema::make("cpython3","3.11");
    C(pk.runtimeId=="cpython3","id");P();}
void t2(){T(version_set);
    auto pk=RuntimeSemanticsPackSchema::make("jvm11","11");
    C(pk.version=="11","ver");P();}
void t3(){T(non_empty_loaded);
    auto pk=RuntimeSemanticsPackSchema::make("cpython3","3.11");
    C(pk.loaded,"loaded");P();}
void t4(){T(empty_id_not_loaded);
    auto pk=RuntimeSemanticsPackSchema::make("","3.11");
    C(!pk.loaded,"not loaded");P();}
void t5(){T(assumptions_generated);
    auto pk=RuntimeSemanticsPackSchema::make("cpython3","3.11");
    C(!pk.assumptions.empty(),"assumptions");P();}
void t6(){T(validate_valid);
    auto pk=RuntimeSemanticsPackSchema::make("cpython3","3.11");
    C(RuntimeSemanticsPackSchema::validate(pk),"valid");P();}
void t7(){T(validate_empty_id_fails);
    RuntimeSemanticsPack pk; pk.version="3.11";
    std::string err;
    C(!RuntimeSemanticsPackSchema::validate(pk,&err)&&err=="runtime_id_missing","err");P();}
void t8(){T(validate_empty_version_fails);
    RuntimeSemanticsPack pk; pk.runtimeId="jvm11";
    std::string err;
    C(!RuntimeSemanticsPackSchema::validate(pk,&err)&&err=="version_missing","err");P();}
void t9(){T(assumption_has_category);
    auto pk=RuntimeSemanticsPackSchema::make("cpython3","3.11");
    C(!pk.assumptions[0].category.empty(),"cat");P();}
void t10(){T(to_json_has_runtime_id);
    auto pk=RuntimeSemanticsPackSchema::make("cpython3","3.11");
    auto j=RuntimeSemanticsPackSchema::toJson(pk);
    C(j.contains("runtime_id"),"json");P();}
void t11(){T(to_json_assumptions_array);
    auto pk=RuntimeSemanticsPackSchema::make("cpython3","3.11");
    auto j=RuntimeSemanticsPackSchema::toJson(pk);
    C(j["assumptions"].is_array(),"arr");P();}
void t12(){T(to_json_loaded_field);
    auto pk=RuntimeSemanticsPackSchema::make("cpython3","3.11");
    auto j=RuntimeSemanticsPackSchema::toJson(pk);
    C(j.contains("loaded"),"loaded");P();}

int main(){
    std::cout<<"Step 889: Runtime semantics pack schema\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
