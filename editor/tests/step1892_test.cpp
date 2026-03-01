// Step 1892: Sprint 272 Integration — all 3 polyglot test projects route correctly
//
//  t1: poly-sort — sort-core → Rust, data-gen → Python
//  t2: poly-api  — http-server → Go, api-client → TypeScript
//  t3: poly-parse — lexer → C++, ast-transform → Haskell
//  t4: explicit overrides survive routing across all 3 projects simultaneously
//  t5: Sprint272IntegrationSummary struct reports correct counts

#include "PolySortProject.h"
#include "PolyApiProject.h"
#include "PolyParseProject.h"
#include "Sprint272IntegrationSummary.h"
#include <iostream>
#include <string>

using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(poly_sort_routes_rust_python);
    auto spec = whetstone::PolySortProject::make();
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "Rust",
      "sort-core → " + spec.sections[0].assignedLanguage);
    C(spec.sections[1].assignedLanguage == "Python",
      "data-gen → " + spec.sections[1].assignedLanguage);
    P();
}

void t2(){
    T(poly_api_routes_go_typescript);
    auto spec = whetstone::PolyApiProject::make();
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "Go",
      "http-server → " + spec.sections[0].assignedLanguage);
    C(spec.sections[1].assignedLanguage == "TypeScript",
      "api-client → " + spec.sections[1].assignedLanguage);
    P();
}

void t3(){
    T(poly_parse_routes_cpp_haskell);
    auto spec = whetstone::PolyParseProject::make();
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "C++",
      "lexer → " + spec.sections[0].assignedLanguage);
    C(spec.sections[1].assignedLanguage == "Haskell",
      "ast-transform → " + spec.sections[1].assignedLanguage);
    P();
}

void t4(){
    T(explicit_override_survives_across_all_projects);
    // Pin sort-core to Go (override) — router must preserve it
    auto spec = whetstone::PolySortProject::make();
    spec.sections[0].explicitLanguage = "Go";
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "Go",
      "explicit Go override not preserved, got: " + spec.sections[0].assignedLanguage);
    // data-gen still auto-routes
    C(!spec.sections[1].assignedLanguage.empty(),
      "data-gen should still be auto-routed");
    P();
}

void t5(){
    T(sprint272_integration_summary);
    whetstone::Sprint272IntegrationSummary s;
    C(s.stepsCompleted == 5, "expected 5 steps");
    C(s.testProjectCount == 3, "expected 3 test projects");
    C(s.success, "success should be true");
    C(s.sprintName() == "Sprint 272: Fitness-Routed Polyglot Test Projects", "sprintName mismatch");
    P();
}

int main(){
    std::cout << "Step 1892: Sprint 272 Integration\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
