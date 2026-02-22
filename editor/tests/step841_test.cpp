// Step 841: CertificationEvidenceArchive tests (10 tests)
#include "graduation/CertificationEvidenceArchive.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(build);
    EvidenceRecord rec{"R1","py->cpp","C1","/path","chk","now"};
    auto a=CertificationEvidenceArchive::build("ARC1",{rec});
    C(a.archiveId=="ARC1","archiveId");P();}

void t2(){T(totalRecords);
    std::vector<EvidenceRecord> recs={
        {"R1","a","c1","/p1","c","now"},
        {"R2","b","c1","/p2","c","now"}};
    auto a=CertificationEvidenceArchive::build("ARC1",recs);
    C(a.totalRecords==2,"totalRecords=2");P();}

void t3(){T(version_v2);
    auto a=CertificationEvidenceArchive::build("ARC1",{});
    C(a.version=="v2","version v2");P();}

void t4(){T(archiveId_preserved);
    auto a=CertificationEvidenceArchive::build("MY-ARCHIVE",{});
    C(a.archiveId=="MY-ARCHIVE","archiveId");P();}

void t5(){T(record_fields);
    EvidenceRecord rec{"R1","py->cpp","C1","/path","sha256","now"};
    auto a=CertificationEvidenceArchive::build("ARC1",{rec});
    C(a.records[0].checksum=="sha256","checksum");P();}

void t6(){T(toJson_output);
    auto a=CertificationEvidenceArchive::build("ARC1",{});
    auto j=CertificationEvidenceArchive::toJson(a);
    C(j.contains("archive_id"),"archive_id");C(j.contains("version"),"version");P();}

void t7(){T(toJson_total);
    EvidenceRecord rec{"R1","a","c","/p","c","now"};
    auto a=CertificationEvidenceArchive::build("ARC1",{rec});
    auto j=CertificationEvidenceArchive::toJson(a);
    C(j["total"]==1,"total=1");P();}

void t8(){T(empty_archive);
    auto a=CertificationEvidenceArchive::build("ARC1",{});
    C(a.totalRecords==0,"totalRecords=0");P();}

void t9(){T(validate_no_archiveId_fails);
    EvidenceArchiveV2 a;
    std::string err;
    C(!CertificationEvidenceArchive::validate(a,&err),"validate fails");
    C(err=="archive_id_missing","err");P();}

void t10(){T(validate_success);
    auto a=CertificationEvidenceArchive::build("ARC1",{});
    C(CertificationEvidenceArchive::validate(a),"validates ok");P();}

int main(){
    std::cout<<"Step 841: CertificationEvidenceArchive\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
