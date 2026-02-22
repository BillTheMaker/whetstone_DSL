// Step 697: Sprint 46 integration summary (8 tests)

#include "Sprint46IntegrationSummary.h"

#include <iostream>

static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

void t1() { T(struct_constructable); auto r = Sprint46IntegrationSummary::run(); C(r.stepStart == 689, "not constructable"); P(); }
void t2() { T(all_modules_reachable); auto r = Sprint46IntegrationSummary::run(); C(r.allModulesReachable, "modules unreachable"); P(); }
void t3() { T(tools_listed); auto r = Sprint46IntegrationSummary::run(); C(r.languageMatrixToolListed && r.portingContractToolListed, "tools not listed"); P(); }
void t4() { T(step_range_correct); auto r = Sprint46IntegrationSummary::run(); C(r.stepStart == 689 && r.stepEnd == 697, "step range wrong"); P(); }
void t5() { T(files_list_populated); auto r = Sprint46IntegrationSummary::run(); C(!r.filesAdded.empty(), "files list empty"); P(); }
void t6() { T(success_flag_true); auto r = Sprint46IntegrationSummary::run(); C(r.success, "success false"); P(); }
void t7() { T(deterministic_summary_ordering); auto a = Sprint46IntegrationSummary::run(); auto b = Sprint46IntegrationSummary::run(); C(a.filesAdded == b.filesAdded, "files ordering nondeterministic"); P(); }
void t8() { T(regression_marker_emitted); auto r = Sprint46IntegrationSummary::run(); C(r.regressionMarkerEmitted && r.regressionMarker == "sprint46_foundation_ready", "marker missing"); P(); }

int main() {
    std::cout << "Step 697: Sprint 46 integration summary\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
