// Step 1888: PolyglotProjectSpec + PolyglotFitnessRouter
//
//  t1: construct PolyglotProjectSpec with 2 sections and 1 interface
//  t2: explicit language on a section is preserved by router
//  t3: unassigned section gets fitness-routed (non-empty assignedLanguage)
//  t4: poly-sort spec routes Rust to sort-core (tree-recursive, shared-mem, generics)
//  t5: poly-api spec routes Go to http-server (flat-loop, channels, blocking I/O)

#include "PolyglotProjectSpec.h"
#include <iostream>
#include <string>

using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(construct_polyglot_project_spec);
    whetstone::PolyglotProjectSpec spec;
    spec.projectName = "poly-sort";
    whetstone::PolyglotSection sec1;
    sec1.componentName = "sort-core";
    whetstone::PolyglotSection sec2;
    sec2.componentName = "data-gen";
    spec.sections.push_back(sec1);
    spec.sections.push_back(sec2);
    whetstone::PolyglotInterface iface;
    iface.fromComponent = "data-gen";
    iface.toComponent   = "sort-core";
    iface.description   = "raw integers slice";
    spec.interfaces.push_back(iface);
    C(spec.projectName == "poly-sort", "project name mismatch");
    C(spec.sections.size() == 2, "expected 2 sections");
    C(spec.interfaces.size() == 1, "expected 1 interface");
    C(spec.interfaces[0].description == "raw integers slice", "interface description mismatch");
    P();
}

void t2(){
    T(explicit_language_preserved_by_router);
    whetstone::PolyglotProjectSpec spec;
    spec.projectName = "explicit-test";
    whetstone::PolyglotSection sec;
    sec.componentName   = "forced-cpp";
    sec.explicitLanguage = "C++";
    spec.sections.push_back(sec);
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "C++",
      "explicit language not preserved, got: " + spec.sections[0].assignedLanguage);
    P();
}

void t3(){
    T(unassigned_section_gets_fitness_routed);
    whetstone::PolyglotProjectSpec spec;
    spec.projectName = "auto-route-test";
    whetstone::PolyglotSection sec;
    sec.componentName = "worker";
    // features left at defaults (all None, mutation=0) — should still pick something
    spec.sections.push_back(sec);
    whetstone::PolyglotFitnessRouter::route(spec);
    C(!spec.sections[0].assignedLanguage.empty(),
      "assignedLanguage should be non-empty after auto-routing");
    P();
}

void t4(){
    T(poly_sort_routes_rust_to_sort_core);
    // sort-core: tree-recursive, shared-memory, async, simple-generics, low mutation
    whetstone::PolyglotProjectSpec spec;
    spec.projectName = "poly-sort";
    whetstone::PolyglotSection sortCore;
    sortCore.componentName = "sort-core";
    sortCore.features.mutationRatio         = 0.15f;
    sortCore.features.recursionShape        = AFF::RecursionShape::TreeRecursive;
    sortCore.features.concurrencyPrimitive  = AFF::ConcurrencyPrimitive::SharedMemory;
    sortCore.features.ioPattern             = AFF::IOPattern::Async;
    sortCore.features.typeComplexity        = AFF::TypeComplexity::SimpleGenerics;
    spec.sections.push_back(sortCore);
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "Rust",
      "sort-core should route to Rust, got: " + spec.sections[0].assignedLanguage);
    P();
}

void t5(){
    T(poly_api_routes_go_to_http_server);
    // http-server: flat-loop, channels, blocking I/O, moderate mutation, no type complexity
    whetstone::PolyglotProjectSpec spec;
    spec.projectName = "poly-api";
    whetstone::PolyglotSection httpServer;
    httpServer.componentName = "http-server";
    httpServer.features.mutationRatio         = 0.30f;
    httpServer.features.recursionShape        = AFF::RecursionShape::FlatLoop;
    httpServer.features.concurrencyPrimitive  = AFF::ConcurrencyPrimitive::Channels;
    httpServer.features.ioPattern             = AFF::IOPattern::Blocking;
    httpServer.features.typeComplexity        = AFF::TypeComplexity::None;
    spec.sections.push_back(httpServer);
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "Go",
      "http-server should route to Go, got: " + spec.sections[0].assignedLanguage);
    P();
}

int main(){
    std::cout << "Step 1888: PolyglotProjectSpec + PolyglotFitnessRouter\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
