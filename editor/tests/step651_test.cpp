// Step 651: Language generator plugin manifest (12 tests)

#include "LanguageGeneratorPluginManifest.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static PluginManifest good(){return {"RustGen","whetstone-plugin-rust.so","RustGenerator",{"rust"}};}

void t1(){T(valid_manifest_passes);C(LanguageGeneratorPluginManifest::isValid(good()),"valid expected");P();}
void t2(){T(invalid_missing_plugin_name);auto m=good();m.pluginName="";C(!LanguageGeneratorPluginManifest::isValid(m),"invalid expected");P();}
void t3(){T(invalid_missing_shared_object);auto m=good();m.sharedObject="";C(!LanguageGeneratorPluginManifest::isValid(m),"invalid expected");P();}
void t4(){T(invalid_missing_generator_class);auto m=good();m.generatorClass="";C(!LanguageGeneratorPluginManifest::isValid(m),"invalid expected");P();}
void t5(){T(invalid_missing_languages);auto m=good();m.languages.clear();C(!LanguageGeneratorPluginManifest::isValid(m),"invalid expected");P();}
void t6(){T(plugin_file_pattern_accepts_valid_name);C(LanguageGeneratorPluginManifest::isPluginFile("whetstone-plugin-rust.so"),"should pass");P();}
void t7(){T(plugin_file_pattern_rejects_wrong_prefix);C(!LanguageGeneratorPluginManifest::isPluginFile("plugin-rust.so"),"should fail");P();}
void t8(){T(plugin_file_pattern_rejects_wrong_suffix);C(!LanguageGeneratorPluginManifest::isPluginFile("whetstone-plugin-rust.dll"),"should fail");P();}
void t9(){T(symbol_name_uses_generator_class);C(LanguageGeneratorPluginManifest::symbolName(good())=="create_RustGenerator","symbol mismatch");P();}
void t10(){T(symbol_name_allows_custom_class);auto m=good();m.generatorClass="GoGenerator";C(LanguageGeneratorPluginManifest::symbolName(m)=="create_GoGenerator","symbol mismatch");P();}
void t11(){T(plugin_file_detection_handles_short_names);C(!LanguageGeneratorPluginManifest::isPluginFile(".so"),"short should fail");P();}
void t12(){T(plugin_file_detection_handles_empty_name);C(!LanguageGeneratorPluginManifest::isPluginFile(""),"empty should fail");P();}

int main(){std::cout<<"Step 651: language generator plugin manifest\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
