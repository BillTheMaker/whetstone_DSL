// Step 1893: ABIBoundaryExtractor
// Identifies AST nodes that cross language boundaries between polyglot components.
// Semantic: toComponent is the provider/library; its exported symbols form the boundary.
//
//  t1: exported function from sort-core (Rust, toComponent) found for data-gen→sort-core iface
//  t2: non-exported nodes on toComponent are excluded
//  t3: struct kind exported by provider is extracted correctly
//  t4: multiple interfaces — each provider's exported nodes collected independently
//  t5: provider (toComponent) absent from AST → zero nodes, no throw

#include "ABIBoundaryExtractor.h"
#include "PolySortProject.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace ws = whetstone;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

// Build AST JSON: {"components": {"comp": {"nodes": [{name,kind,exported},...]}}}
static json buildAST(std::vector<std::pair<std::string,
                     std::vector<std::tuple<std::string,std::string,bool>>>> comps) {
    json ast;
    ast["components"] = json::object();
    for (auto& [comp, nodes] : comps) {
        json nodeArr = json::array();
        for (auto& [name, kind, exp] : nodes) {
            nodeArr.push_back({{"name", name}, {"kind", kind}, {"exported", exp}});
        }
        ast["components"][comp] = {{"nodes", nodeArr}};
    }
    return ast;
}

void t1(){
    T(exported_function_from_provider_rust_extracted);
    // PolySortProject interface: from=data-gen (Python), to=sort-core (Rust)
    // sort-core is the provider — it exports sort_array
    auto spec = ws::PolySortProject::make();
    ws::PolyglotFitnessRouter::route(spec);

    json ast = buildAST({
        {"sort-core", {{"sort_array",    "function", true},
                       {"internal_swap", "function", false}}},
        {"data-gen",  {{"generate_data", "function", true}}}
    });

    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);
    bool found = false;
    for (auto& n : nodes) {
        if (n.name == "sort_array" &&
            n.fromComponent == "data-gen" && n.toComponent == "sort-core") {
            found = true;
            C(n.fromLanguage == "Python",
              "fromLanguage should be Python (data-gen), got: " + n.fromLanguage);
            C(n.toLanguage == "Rust",
              "toLanguage should be Rust (sort-core), got: " + n.toLanguage);
            C(n.kind == "function",
              "kind should be function, got: " + n.kind);
        }
    }
    C(found, "sort_array boundary node not found");
    P();
}

void t2(){
    T(non_exported_nodes_on_provider_excluded);
    // interface: data-gen → sort-core; sort-core has internal_swap (non-exported) → must be excluded
    auto spec = ws::PolySortProject::make();
    ws::PolyglotFitnessRouter::route(spec);

    json ast = buildAST({
        {"sort-core", {{"internal_swap", "function", false},
                       {"sort_array",    "function", true}}},
        {"data-gen",  {{"generate_data", "function", true}}}
    });

    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);
    for (auto& n : nodes) {
        C(n.name != "internal_swap",
          "non-exported internal_swap must not appear in boundary nodes");
    }
    P();
}

void t3(){
    T(exported_struct_from_provider_extracted_correctly);
    // from=caller (alpha/Rust), to=provider (beta/Python); beta exports SortResult struct
    ws::PolyglotProjectSpec spec;
    spec.projectName = "struct-test";
    spec.sections.push_back({"caller",   {}, "Rust",   "Rust"});
    spec.sections.push_back({"provider", {}, "Python", "Python"});
    spec.interfaces.push_back({"caller", "provider",
                                "caller uses SortResult from provider"});

    json ast = buildAST({
        {"caller",   {{"call_fn",       "function", true}}},
        {"provider", {{"SortResult",    "struct",   true},
                      {"internal_state","struct",   false}}}
    });

    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);
    bool found = false;
    for (auto& n : nodes) {
        if (n.name == "SortResult") {
            found = true;
            C(n.kind == "struct",
              "kind should be struct, got: " + n.kind);
            C(n.fromComponent == "caller",
              "fromComponent should be caller, got: " + n.fromComponent);
            C(n.toComponent == "provider",
              "toComponent should be provider, got: " + n.toComponent);
        }
    }
    C(found, "SortResult struct boundary node not found");
    P();
}

void t4(){
    T(multiple_interfaces_all_provider_exports_collected);
    // Two callers (beta, gamma) each calling provider alpha
    ws::PolyglotProjectSpec spec;
    spec.projectName = "multi-iface";
    spec.sections.push_back({"alpha", {}, "Rust",       "Rust"});
    spec.sections.push_back({"beta",  {}, "Python",     "Python"});
    spec.sections.push_back({"gamma", {}, "Go",         "Go"});
    // beta calls alpha, gamma calls alpha
    spec.interfaces.push_back({"beta",  "alpha", "beta uses alpha"});
    spec.interfaces.push_back({"gamma", "alpha", "gamma uses alpha"});

    json ast = buildAST({
        {"alpha", {{"service_fn", "function", true}}},
        {"beta",  {{"caller_b",   "function", true}}},
        {"gamma", {{"caller_c",   "function", true}}}
    });

    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);
    // alpha's service_fn should appear for both interfaces
    int baCount = 0, gaCount = 0;
    for (auto& n : nodes) {
        if (n.name == "service_fn" &&
            n.fromComponent == "beta"  && n.toComponent == "alpha") ++baCount;
        if (n.name == "service_fn" &&
            n.fromComponent == "gamma" && n.toComponent == "alpha") ++gaCount;
    }
    C(baCount == 1, "expected service_fn node for beta→alpha");
    C(gaCount == 1, "expected service_fn node for gamma→alpha");
    P();
}

void t5(){
    T(missing_provider_in_ast_produces_no_nodes_no_throw);
    // interface: caller → provider; provider absent from AST
    ws::PolyglotProjectSpec spec;
    spec.projectName = "missing-test";
    spec.sections.push_back({"caller",   {}, "Rust",   "Rust"});
    spec.sections.push_back({"provider", {}, "Python", "Python"});
    spec.interfaces.push_back({"caller", "provider", "caller→provider"});

    // AST has caller but NOT provider (the toComponent)
    json ast = buildAST({
        {"caller", {{"call_fn", "function", true}}}
    });

    std::vector<ws::ABIBoundaryNode> nodes;
    try {
        nodes = ws::ABIBoundaryExtractor::extract(spec, ast);
    } catch (...) {
        F("extract threw an exception for missing provider component");
        return;
    }
    C(nodes.empty(), "expected zero boundary nodes when provider is absent from AST");
    P();
}

int main(){
    std::cout << "Step 1893: ABIBoundaryExtractor\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
