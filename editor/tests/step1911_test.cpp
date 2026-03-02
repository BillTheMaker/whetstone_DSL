// Step 1911: CrossLanguageDefinitionResolver
// Resolves goto-definition for cross-language boundary symbols using the
// CrossLanguageSymbolTable and SCIP index. Returns a location pointing
// to the provider component's source.
//
//  t1: definition of a boundary symbol returns provider location
//  t2: definition of unknown symbol returns not-found
//  t3: scipSymbol path in resolved location matches table record
//  t4: resolving from Python call site lands on Rust provider
//  t5: multiple boundaries — correct provider returned for each

#include "CrossLanguageDefinitionResolver.h"
#include "CrossLanguageSymbolTable.h"
#include "ABIBoundaryExtractor.h"
#include <iostream>
#include <string>

namespace ws = whetstone;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static ws::ABIBoundaryNode makeNode(const std::string& name,
                                    const std::string& from, const std::string& to,
                                    const std::string& fromLang, const std::string& toLang) {
    ws::ABIBoundaryNode n;
    n.name          = name;
    n.kind          = "function";
    n.fromComponent = from;
    n.toComponent   = to;
    n.fromLanguage  = fromLang;
    n.toLanguage    = toLang;
    n.signature     = "void " + name + "()";
    return n;
}

void t1(){
    T(boundary_symbol_resolves_to_provider_location);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));

    ws::CrossLanguageDefinitionResolver resolver(table);
    auto loc = resolver.resolve("sort_array");
    C(loc.found, "sort_array must resolve");
    C(loc.providerComponent == "sort-core", "provider must be sort-core");
    C(loc.providerLanguage  == "Rust",      "provider language must be Rust");
    C(!loc.scipSymbol.empty(), "scipSymbol must be non-empty");
    P();
}

void t2(){
    T(unknown_symbol_returns_not_found);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));

    ws::CrossLanguageDefinitionResolver resolver(table);
    auto loc = resolver.resolve("nonexistent");
    C(!loc.found, "nonexistent must not resolve");
    P();
}

void t3(){
    T(scip_symbol_path_matches_record);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));

    ws::CrossLanguageDefinitionResolver resolver(table);
    auto loc = resolver.resolve("sort_array");
    C(loc.found, "must resolve");
    // scipSymbol format: "<toComponent>/<name>."
    C(loc.scipSymbol == "sort-core/sort_array.", "scipSymbol must be sort-core/sort_array.");
    P();
}

void t4(){
    T(python_call_site_resolves_to_rust_provider);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));

    ws::CrossLanguageDefinitionResolver resolver(table);
    // Caller is in Python; definition should land in Rust provider
    auto loc = resolver.resolveFromUri("file:///src/main.py", "sort_array");
    C(loc.found, "must resolve from Python context");
    C(loc.providerLanguage == "Rust", "provider language must be Rust");
    C(loc.callerLanguage   == "Python", "caller language must be Python");
    P();
}

void t5(){
    T(multiple_boundaries_correct_provider_returned);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));
    table.insert(makeNode("compress","encoder","codec","Go","C++"));
    table.insert(makeNode("render","app","gpu-core","TypeScript","C++"));

    ws::CrossLanguageDefinitionResolver resolver(table);
    auto r1 = resolver.resolve("compress");
    C(r1.found && r1.providerComponent == "codec", "compress→codec");
    auto r2 = resolver.resolve("render");
    C(r2.found && r2.providerComponent == "gpu-core", "render→gpu-core");
    auto r3 = resolver.resolve("sort_array");
    C(r3.found && r3.providerComponent == "sort-core", "sort_array→sort-core");
    P();
}

int main(){
    std::cout << "Step 1911: CrossLanguageDefinitionResolver\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
