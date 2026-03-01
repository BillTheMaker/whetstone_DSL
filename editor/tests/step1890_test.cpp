// Step 1890: poly-api test project spec — Go http-server + TypeScript api-client
//
//  t1: PolyApiProject::make() produces correct project name and structure
//  t2: http-server section features match expected values
//  t3: api-client section features match expected values
//  t4: interface is from api-client to http-server
//  t5: after routing, http-server → Go and api-client → TypeScript

#include "PolyApiProject.h"
#include <iostream>
#include <string>

using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(poly_api_project_structure);
    auto spec = whetstone::PolyApiProject::make();
    C(spec.projectName == "poly-api", "wrong project name: " + spec.projectName);
    C(spec.sections.size() == 2, "expected 2 sections, got " + std::to_string(spec.sections.size()));
    C(spec.interfaces.size() == 1, "expected 1 interface");
    C(spec.sections[0].componentName == "http-server", "first section should be http-server");
    C(spec.sections[1].componentName == "api-client", "second section should be api-client");
    P();
}

void t2(){
    T(http_server_features_correct);
    auto spec = whetstone::PolyApiProject::make();
    const auto& f_ = spec.sections[0].features;
    C(f_.mutationRatio > 0.25f && f_.mutationRatio < 0.35f,
      "http-server mutationRatio expected ~0.30, got " + std::to_string(f_.mutationRatio));
    C(f_.recursionShape == AFF::RecursionShape::FlatLoop,
      "http-server recursionShape should be FlatLoop");
    C(f_.concurrencyPrimitive == AFF::ConcurrencyPrimitive::Channels,
      "http-server concurrencyPrimitive should be Channels");
    C(f_.ioPattern == AFF::IOPattern::Blocking,
      "http-server ioPattern should be Blocking");
    C(f_.typeComplexity == AFF::TypeComplexity::None,
      "http-server typeComplexity should be None");
    P();
}

void t3(){
    T(api_client_features_correct);
    auto spec = whetstone::PolyApiProject::make();
    const auto& f_ = spec.sections[1].features;
    C(f_.mutationRatio > 0.15f && f_.mutationRatio < 0.25f,
      "api-client mutationRatio expected ~0.20, got " + std::to_string(f_.mutationRatio));
    C(f_.recursionShape == AFF::RecursionShape::FlatLoop,
      "api-client recursionShape should be FlatLoop");
    C(f_.concurrencyPrimitive == AFF::ConcurrencyPrimitive::None,
      "api-client concurrencyPrimitive should be None");
    C(f_.ioPattern == AFF::IOPattern::Async,
      "api-client ioPattern should be Async");
    C(f_.typeComplexity == AFF::TypeComplexity::SimpleGenerics,
      "api-client typeComplexity should be SimpleGenerics");
    P();
}

void t4(){
    T(interface_api_client_to_http_server);
    auto spec = whetstone::PolyApiProject::make();
    C(spec.interfaces[0].fromComponent == "api-client", "interface.from should be api-client");
    C(spec.interfaces[0].toComponent   == "http-server", "interface.to should be http-server");
    C(spec.interfaces[0].description   == "JSON REST calls", "interface description mismatch");
    P();
}

void t5(){
    T(routing_http_server_go_api_client_typescript);
    auto spec = whetstone::PolyApiProject::make();
    whetstone::PolyglotFitnessRouter::route(spec);
    C(spec.sections[0].assignedLanguage == "Go",
      "http-server should route to Go, got: " + spec.sections[0].assignedLanguage);
    C(spec.sections[1].assignedLanguage == "TypeScript",
      "api-client should route to TypeScript, got: " + spec.sections[1].assignedLanguage);
    P();
}

int main(){
    std::cout << "Step 1890: poly-api test project (Go http-server + TypeScript api-client)\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
