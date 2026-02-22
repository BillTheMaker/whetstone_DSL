// Step 836: DocumentationPackGenerator tests (8 tests)
#include "graduation/DocumentationPackGenerator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(build_with_data);
    PlaybookEntry e{"py->cpp","oop","step1,step2","none"};
    auto p2=DocumentationPackGenerator::build("v1.0",{e},{"py->cpp"});
    C(p2.version=="v1.0","version");P();}

void t2(){T(published_flag);
    PlaybookEntry e{"py->cpp","oop","steps","none"};
    auto p2=DocumentationPackGenerator::build("v1.0",{e},{"py->cpp"});
    C(p2.published,"published");P();}

void t3(){T(empty_version_unpublished);
    auto p2=DocumentationPackGenerator::build("",{},{"py->cpp"});
    C(!p2.published,"not published");P();}

void t4(){T(playbooks_preserved);
    PlaybookEntry e{"py->cpp","oop","steps","none"};
    auto p2=DocumentationPackGenerator::build("v1.0",{e},{"py->cpp"});
    C(p2.playbooks.size()==1,"playbooks");P();}

void t5(){T(pairs_list);
    auto p2=DocumentationPackGenerator::build("v1.0",{},{"py->cpp","rust->cpp"});
    C(p2.supportedPairs.size()==2,"pairs");P();}

void t6(){T(toJson_output);
    auto p2=DocumentationPackGenerator::build("v1.0",{},{"py->cpp"});
    auto j=DocumentationPackGenerator::toJson(p2);
    C(j.contains("version")&&j.contains("published"),"keys");P();}

void t7(){T(empty_pairs_unpublished);
    auto p2=DocumentationPackGenerator::build("v1.0",{},{});
    C(!p2.published,"not published");P();}

void t8(){T(toJson_supported_pairs);
    auto p2=DocumentationPackGenerator::build("v1.0",{},{"py->cpp"});
    auto j=DocumentationPackGenerator::toJson(p2);
    C(j.contains("supported_pairs"),"supported_pairs");P();}

int main(){
    std::cout<<"Step 836: DocumentationPackGenerator\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
