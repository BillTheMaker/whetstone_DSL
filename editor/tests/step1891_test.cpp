// Step 1891: poly-parse test project spec — C++ lexer + Haskell ast-transform
//
//  t1: PolyParseProject::make() produces correct project name and structure
//  t2: lexer section features match expected values
//  t3: ast-transform section features match expected values
//  t4: interface is from lexer to ast-transform
//  t5: after routing, lexer → C++ and ast-transform → Haskell

#include "PolyParseProject.h"
#include <iostream>
#include <string>

using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(poly_parse_project_structure);
    auto spec = whetstone::PolyParseProject::make();
    C(spec.projectName == "poly-parse", "wrong project name: " + spec.projectName);
    C(spec.sections.size() == 2, "expected 2 sections, got " + std::to_string(spec.sections.size()));
    C(spec.interfaces.size() == 1, "expected 1 interface");
    C(spec.sections[0].componentName == "lexer", "first section should be lexer");
    C(spec.sections[1].componentName == "ast-transform", "second section should be ast-transform");
    P();
}

void t2(){
    T(lexer_features_correct);
    auto spec = whetstone::PolyParseProject::make();
    const auto& f_ = spec.sections[0].features;
    C(f_.mutationRatio > 0.30f && f_.mutationRatio < 0.40f,
      "lexer mutationRatio expected ~0.35, got " + std::to_string(f_.mutationRatio));
    C(f_.recursionShape == AFF::RecursionShape::TreeRecursive,
      "lexer recursionShape should be TreeRecursive");
    C(f_.concurrencyPrimitive == AFF::ConcurrencyPrimitive::SharedMemory,
      "lexer concurrencyPrimitive should be SharedMemory");
    C(f_.ioPattern == AFF::IOPattern::Blocking,
      "lexer ioPattern should be Blocking");
    C(f_.typeComplexity == AFF::TypeComplexity::SimpleGenerics,
      "lexer typeComplexity should be SimpleGenerics");
    P();
}

void t3(){
    T(ast_transform_features_correct);
    auto spec = whetstone::PolyParseProject::make();
    const auto& f_ = spec.sections[1].features;
    C(f_.mutationRatio == 0.0f,
      "ast-transform mutationRatio should be 0, got " + std::to_string(f_.mutationRatio));
    C(f_.recursionShape == AFF::RecursionShape::TailRecursive,
      "ast-transform recursionShape should be TailRecursive");
    C(f_.concurrencyPrimitive == AFF::ConcurrencyPrimitive::None,
      "ast-transform concurrencyPrimitive should be None");
    C(f_.ioPattern == AFF::IOPattern::None,
      "ast-transform ioPattern should be None");
    C(f_.typeComplexity == AFF::TypeComplexity::DependentTypes,
      "ast-transform typeComplexity should be DependentTypes");
    P();
}

void t4(){
    T(interface_lexer_to_ast_transform);
    auto spec = whetstone::PolyParseProject::make();
    C(spec.interfaces[0].fromComponent == "lexer", "interface.from should be lexer");
    C(spec.interfaces[0].toComponent   == "ast-transform", "interface.to should be ast-transform");
    C(spec.interfaces[0].description   == "token stream", "interface description mismatch");
    P();
}

void t5(){
    T(routing_lexer_cpp_ast_transform_haskell);
    auto spec = whetstone::PolyParseProject::make();
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "C++",
      "lexer should route to C++, got: " + spec.sections[0].assignedLanguage);
    C(spec.sections[1].assignedLanguage == "Haskell",
      "ast-transform should route to Haskell, got: " + spec.sections[1].assignedLanguage);
    P();
}

int main(){
    std::cout << "Step 1891: poly-parse test project (C++ lexer + Haskell ast-transform)\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
