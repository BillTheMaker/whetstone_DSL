// Step 1880 TDD: GR-009 — CrossFileTransactionGate enforces multi-file atomic promotion
//
// DeploymentPromotionGate tracked single environments (files) independently.
// Long-range edits that span multiple files had no atomic gate: one file could
// be blocked while others promoted. CrossFileTransactionGate fixes this.
//
//  t1: all files fully checked + approved → transaction promotable
//  t2: one file has blocking issues → transaction not promotable, that file in blockedFiles
//  t3: empty transaction → promotable (vacuously)
//  t4: one file not fully checked → not promotable
//  t5: mixed: 2 ready + 1 blocked → promotable=false, blockedFiles has exactly 1

#include "CrossFileTransactionGate.h"
#include <iostream>
#include <string>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static FileEditState ready(const std::string& id) {
    return {id, 3, 3, 1, 0};  // 3/3 checks, 1 approval, 0 blocking issues
}

static FileEditState blocked(const std::string& id) {
    return {id, 3, 2, 0, 1};  // 2/3 checks, no approval, 1 blocking issue
}

void t1(){
    T(all_files_ready_transaction_promotable);
    auto result = CrossFileTransactionGate::evaluate({
        ready("src/auth.cpp"),
        ready("src/session.cpp"),
        ready("include/auth.h")
    });
    C(result.promotable, "all files ready → transaction should be promotable");
    C(result.blockedFiles.empty(), "no files should be blocked");
    C(result.readyFiles == 3, "expected 3 ready files");
    P();
}

void t2(){
    T(one_blocked_file_blocks_transaction);
    auto result = CrossFileTransactionGate::evaluate({
        ready("src/auth.cpp"),
        blocked("src/session.cpp")
    });
    C(!result.promotable, "one blocked file should block the transaction");
    C(result.blockedFiles.size() == 1u, "expected 1 blocked file");
    C(result.blockedFiles[0] == "src/session.cpp", "expected session.cpp as blocked");
    P();
}

void t3(){
    T(empty_transaction_promotable);
    auto result = CrossFileTransactionGate::evaluate({});
    C(result.promotable, "empty transaction should be vacuously promotable");
    C(result.totalFiles == 0, "expected 0 total files");
    P();
}

void t4(){
    T(partially_checked_file_not_promotable);
    FileEditState partial{"src/db.cpp", 4, 2, 1, 0};  // 2/4 checks — not complete
    auto result = CrossFileTransactionGate::evaluate({partial});
    C(!result.promotable, "partially checked file should not be promotable");
    C(result.blockedFiles.size() == 1u, "expected 1 blocked file");
    P();
}

void t5(){
    T(two_ready_one_blocked);
    auto result = CrossFileTransactionGate::evaluate({
        ready("a.cpp"),
        ready("b.cpp"),
        blocked("c.cpp")
    });
    C(!result.promotable, "transaction should not promote with 1 blocked file");
    C(result.readyFiles == 2, "expected 2 ready files, got " + std::to_string(result.readyFiles));
    C(result.blockedFiles.size() == 1u, "expected exactly 1 blocked file");
    P();
}

int main(){
    std::cout<<"Step 1880: GR-009 CrossFileTransactionGate multi-file atomic promotion\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
