// Step 1889: poly-sort test project spec — Rust sort-core + Python data-gen
//
//  t1: PolySortProject::make() produces correct project name and structure
//  t2: sort-core section features match expected values
//  t3: data-gen section features match expected values
//  t4: interface is from data-gen to sort-core
//  t5: after routing, sort-core → Rust and data-gen → Python

#include "PolySortProject.h"
#include <iostream>
#include <string>

using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(poly_sort_project_structure);
    auto spec = whetstone::PolySortProject::make();
    C(spec.projectName == "poly-sort", "wrong project name: " + spec.projectName);
    C(spec.sections.size() == 2, "expected 2 sections, got " + std::to_string(spec.sections.size()));
    C(spec.interfaces.size() == 1, "expected 1 interface, got " + std::to_string(spec.interfaces.size()));
    C(spec.sections[0].componentName == "sort-core", "first section should be sort-core");
    C(spec.sections[1].componentName == "data-gen", "second section should be data-gen");
    P();
}

void t2(){
    T(sort_core_features_correct);
    auto spec = whetstone::PolySortProject::make();
    const auto& f_ = spec.sections[0].features;
    C(f_.mutationRatio > 0.10f && f_.mutationRatio < 0.20f,
      "sort-core mutationRatio expected ~0.15, got " + std::to_string(f_.mutationRatio));
    C(f_.recursionShape == AFF::RecursionShape::TreeRecursive,
      "sort-core recursionShape should be TreeRecursive");
    C(f_.concurrencyPrimitive == AFF::ConcurrencyPrimitive::SharedMemory,
      "sort-core concurrencyPrimitive should be SharedMemory");
    C(f_.ioPattern == AFF::IOPattern::Async,
      "sort-core ioPattern should be Async");
    C(f_.typeComplexity == AFF::TypeComplexity::SimpleGenerics,
      "sort-core typeComplexity should be SimpleGenerics");
    P();
}

void t3(){
    T(data_gen_features_correct);
    auto spec = whetstone::PolySortProject::make();
    const auto& f_ = spec.sections[1].features;
    C(f_.mutationRatio > 0.50f,
      "data-gen mutationRatio expected ~0.55, got " + std::to_string(f_.mutationRatio));
    C(f_.recursionShape == AFF::RecursionShape::FlatLoop,
      "data-gen recursionShape should be FlatLoop");
    C(f_.concurrencyPrimitive == AFF::ConcurrencyPrimitive::None,
      "data-gen concurrencyPrimitive should be None");
    C(f_.ioPattern == AFF::IOPattern::Blocking,
      "data-gen ioPattern should be Blocking");
    C(f_.typeComplexity == AFF::TypeComplexity::None,
      "data-gen typeComplexity should be None");
    P();
}

void t4(){
    T(interface_data_gen_to_sort_core);
    auto spec = whetstone::PolySortProject::make();
    C(spec.interfaces[0].fromComponent == "data-gen", "interface.from should be data-gen");
    C(spec.interfaces[0].toComponent   == "sort-core", "interface.to should be sort-core");
    C(spec.interfaces[0].description   == "raw integer slice", "interface description mismatch");
    P();
}

void t5(){
    T(routing_sort_core_rust_data_gen_python);
    auto spec = whetstone::PolySortProject::make();
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "Rust",
      "sort-core should route to Rust, got: " + spec.sections[0].assignedLanguage);
    C(spec.sections[1].assignedLanguage == "Python",
      "data-gen should route to Python, got: " + spec.sections[1].assignedLanguage);
    P();
}

int main(){
    std::cout << "Step 1889: poly-sort test project (Rust sort-core + Python data-gen)\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
