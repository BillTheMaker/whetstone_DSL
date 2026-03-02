// Step 1906: whetstone_emit_symbol_index MCP tool wiring
// Tests SymbolIndexOrchestrator logic directly.
// Orchestrator: extract nodes → build table → emit SCIP index → return.
//
//  t1: Python→Rust boundary produces SCIP index with "Rust" and "Python" documents
//  t2: SCIP index contains sort_array symbol with correct scipSymbol
//  t3: table lookup works after orchestration
//  t4: incremental update via orchestrator returns correct diff counts
//  t5: orchestrate() result has required top-level fields

#include "SymbolIndexOrchestrator.h"
#include "ABIBoundaryExtractor.h"
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

static ws::ABIBoundaryNode makeNode(const std::string& name,
                                    const std::string& fromComp, const std::string& toComp,
                                    const std::string& fromLang, const std::string& toLang)
{
    ws::ABIBoundaryNode n;
    n.name = name; n.kind = "function";
    n.fromComponent = fromComp; n.toComponent = toComp;
    n.fromLanguage = fromLang;  n.toLanguage  = toLang;
    n.signature = {{"name",name},{"kind","function"},{"exported",true}};
    return n;
}

void t1(){
    T(python_rust_boundary_produces_rust_and_python_documents);
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    ws::CrossLanguageSymbolTable table;
    auto index = ws::SymbolIndexOrchestrator::orchestrate({node}, table);

    C(index.contains("scip_index"), "result must have scip_index");
    const auto& docs = index["scip_index"]["documents"];
    bool hasRust = false, hasPython = false;
    for (const auto& doc : docs) {
        if (doc["language"] == "Rust")   hasRust   = true;
        if (doc["language"] == "Python") hasPython = true;
    }
    C(hasRust,   "must have Rust document");
    C(hasPython, "must have Python document");
    P();
}

void t2(){
    T(scip_index_contains_sort_array_with_correct_scip_symbol);
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    ws::CrossLanguageSymbolTable table;
    auto index = ws::SymbolIndexOrchestrator::orchestrate({node}, table);

    bool found = false;
    for (const auto& doc : index["scip_index"]["documents"]) {
        for (const auto& sym : doc["symbols"]) {
            if (sym["name"] == "sort_array" &&
                sym["scipSymbol"] == "sort-core/sort_array.") {
                found = true;
            }
        }
    }
    C(found, "sort_array with scipSymbol sort-core/sort_array. not found");
    P();
}

void t3(){
    T(table_lookup_works_after_orchestration);
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    ws::CrossLanguageSymbolTable table;
    ws::SymbolIndexOrchestrator::orchestrate({node}, table);

    auto rec = table.lookup("Rust", "sort_array");
    C(rec.found,                       "lookup must succeed");
    C(rec.toComponent == "sort-core",  "toComponent must be sort-core");
    C(rec.fromLanguage == "Python",    "fromLanguage must be Python");
    P();
}

void t4(){
    T(incremental_update_returns_correct_diff_counts);
    auto n1 = makeNode("fn_a", "alpha", "beta",  "Python", "Rust");
    auto n2 = makeNode("fn_b", "gamma", "delta", "Go",     "C++");

    ws::CrossLanguageSymbolTable table;
    ws::SymbolIndexOrchestrator::orchestrate({n1}, table);

    // Re-generation: n1 stays, n2 added
    auto result = ws::SymbolIndexOrchestrator::update(table, {n1}, {n1, n2});
    C(result.added == 1,     "added must be 1");
    C(result.removed == 0,   "removed must be 0");
    C(result.unchanged == 1, "unchanged must be 1");
    P();
}

void t5(){
    T(orchestrate_result_has_required_fields);
    auto node = makeNode("compute", "a", "b", "TypeScript", "C++");

    ws::CrossLanguageSymbolTable table;
    auto result = ws::SymbolIndexOrchestrator::orchestrate({node}, table);

    C(result.contains("scip_index"),   "must have scip_index");
    C(result.contains("node_count"),   "must have node_count");
    C(result["node_count"] == 1,       "node_count must be 1");
    C(result.contains("success") && result["success"] == true,
      "must have success=true");
    P();
}

int main(){
    std::cout << "Step 1906: SymbolIndexOrchestrator / whetstone_emit_symbol_index\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
