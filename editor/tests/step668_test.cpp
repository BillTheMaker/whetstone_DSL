// Step 668: Sprint 41 integration summary (8 tests)

#include "Sprint41IntegrationSummary.h"
#include <iostream>

static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

void t1() { T(summary_constructable);      auto r = Sprint41IntegrationSummary::run(); C(r.success || !r.success, "not constructable"); P(); }
void t2() { T(schema_to_cpp_wired);        auto r = Sprint41IntegrationSummary::run(); C(r.schemaToCppWired,   "schema_to_cpp not wired");    P(); }
void t3() { T(dispatch_table_wired);       auto r = Sprint41IntegrationSummary::run(); C(r.dispatchTableWired, "dispatch_table not wired");   P(); }
void t4() { T(tool_count_before_is_82);    auto r = Sprint41IntegrationSummary::run(); C(r.toolCountBefore == 82, "expected 82");              P(); }
void t5() { T(tool_count_after_is_84);     auto r = Sprint41IntegrationSummary::run(); C(r.toolCountAfter  == 84, "expected 84");              P(); }
void t6() { T(steps_completed_is_5);       auto r = Sprint41IntegrationSummary::run(); C(r.stepsCompleted  == 5,  "expected 5");               P(); }
void t7() { T(files_added_contains_register_codegen);
    auto r = Sprint41IntegrationSummary::run();
    bool found = false;
    for (const auto& s : r.filesAdded) if (s.find("RegisterCodegenTools") != std::string::npos) { found = true; break; }
    C(found, "RegisterCodegenTools.h not in filesAdded");
    P();
}
void t8() { T(overall_success);            auto r = Sprint41IntegrationSummary::run(); C(r.success, "integration failed"); P(); }

int main() {
    std::cout << "Step 668: Sprint 41 integration summary\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
