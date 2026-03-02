// Step 1907: Sprint 275 Integration
// Full pipeline on poly-sort: extract boundary nodes → build symbol table →
// emit SCIP index → verify cross-language symbol links.
//
//  t1: poly-sort boundary nodes are extracted and inserted into the table
//  t2: sort_array appears in both Python (caller) and Rust (provider) documents
//  t3: lookupByScip("sort-core/sort_array.") returns 2 appearances
//  t4: incremental update: add merge_slices → added=1, unchanged=1
//  t5: Sprint275IntegrationSummary reports correct counts

#include "ABIBoundaryExtractor.h"
#include "SymbolIndexOrchestrator.h"
#include "CrossLanguageSymbolTable.h"
#include "PolySortProject.h"
#include "Sprint275IntegrationSummary.h"
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

// poly-sort AST: interface from=data-gen, to=sort-core; sort-core exports sort_array
static json makePolySortAST() {
    json ast;
    ast["components"]["sort-core"]["nodes"] = json::array({
        {{"name","sort_array"}, {"kind","function"}, {"exported",true},
         {"returnType","void"},
         {"params", json::array({
             {{"name","data"},{"type","i32*"}},{{"name","len"},{"type","usize"}}
         })}}
    });
    ast["components"]["data-gen"]["nodes"] = json::array({
        {{"name","generate"}, {"kind","function"}, {"exported",true}}
    });
    return ast;
}

// Helper: make a standalone boundary node (not from AST extraction)
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
    T(poly_sort_nodes_extracted_and_inserted_into_table);
    auto spec = ws::PolySortProject::make();
    ws::PolyglotFitnessRouter::route(spec);
    auto ast = makePolySortAST();

    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);
    C(!nodes.empty(), "expected boundary nodes from poly-sort");

    ws::CrossLanguageSymbolTable table;
    ws::SymbolIndexOrchestrator::orchestrate(nodes, table);

    // sort_array must be in table under both languages
    bool inProvider = false, inCaller = false;
    for (auto& n : nodes) {
        if (n.name == "sort_array") {
            inProvider = table.contains(n.toLanguage,   "sort_array");
            inCaller   = table.contains(n.fromLanguage, "sort_array");
        }
    }
    C(inProvider, "sort_array not found in provider language");
    C(inCaller,   "sort_array not found in caller language");
    P();
}

void t2(){
    T(sort_array_appears_in_python_and_rust_scip_documents);
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    ws::CrossLanguageSymbolTable table;
    auto result = ws::SymbolIndexOrchestrator::orchestrate({node}, table);

    const auto& docs = result["scip_index"]["documents"];
    bool hasPython = false, hasRust = false;
    for (const auto& doc : docs) {
        std::string lang = doc["language"];
        for (const auto& sym : doc["symbols"]) {
            if (sym["name"] == "sort_array") {
                if (lang == "Python") hasPython = true;
                if (lang == "Rust")   hasRust   = true;
            }
        }
    }
    C(hasPython, "sort_array must appear in Python document");
    C(hasRust,   "sort_array must appear in Rust document");
    P();
}

void t3(){
    T(lookup_by_scip_returns_two_appearances);
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    ws::CrossLanguageSymbolTable table;
    ws::SymbolIndexOrchestrator::orchestrate({node}, table);

    auto apps = table.lookupByScip("sort-core/sort_array.");
    C(apps.size() == 2,
      "expected 2 appearances, got " + std::to_string(apps.size()));
    bool hasCaller = false, hasProvider = false;
    for (auto& a : apps) {
        if (a.role == "caller")   hasCaller   = true;
        if (a.role == "provider") hasProvider = true;
    }
    C(hasCaller,   "must have caller appearance");
    C(hasProvider, "must have provider appearance");
    P();
}

void t4(){
    T(incremental_update_add_merge_slices_correct_diff);
    auto n1 = makeNode("sort_array",   "data-gen", "sort-core", "Python", "Rust");
    auto n2 = makeNode("merge_slices", "data-gen", "sort-core", "Python", "Rust");

    ws::CrossLanguageSymbolTable table;
    ws::SymbolIndexOrchestrator::orchestrate({n1}, table);

    auto diff = ws::SymbolIndexOrchestrator::update(table, {n1}, {n1, n2});
    C(diff.added == 1,     "added must be 1 (merge_slices)");
    C(diff.removed == 0,   "removed must be 0");
    C(diff.unchanged == 1, "unchanged must be 1 (sort_array)");
    C(table.contains("Rust", "merge_slices"), "merge_slices must be in table after update");
    P();
}

void t5(){
    T(sprint275_integration_summary_correct);
    ws::Sprint275IntegrationSummary s;
    C(s.stepsCompleted == 5,    "expected 5 steps");
    C(s.mcpToolsAdded == 1,     "expected 1 MCP tool (whetstone_emit_symbol_index)");
    C(s.success,                "success must be true");
    C(s.sprintName() == "Sprint 275: Cross-Language Symbol Index",
      "sprintName mismatch: " + s.sprintName());
    P();
}

int main(){
    std::cout << "Step 1907: Sprint 275 Integration\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
