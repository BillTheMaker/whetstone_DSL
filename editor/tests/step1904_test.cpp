// Step 1904: CrossLanguageSymbolTable
// In-memory symbol table linking same-origin symbols across all generated languages.
// A symbol originates from one ABIBoundaryNode; it has appearances in multiple languages.
//
//  t1: inserting a node creates entries for both fromLanguage and toLanguage
//  t2: lookup by (language, name) returns the correct symbol record
//  t3: lookup by scipSymbol returns all language appearances of that symbol
//  t4: multiple nodes with different language pairs all inserted correctly
//  t5: lookup for unknown symbol returns empty / not-found result

#include "CrossLanguageSymbolTable.h"
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
    T(inserting_node_creates_entries_for_both_languages);
    ws::CrossLanguageSymbolTable table;
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");
    table.insert(node);

    // Must appear in Python (caller) and Rust (provider)
    C(table.contains("Python", "sort_array"),
      "sort_array must be findable in Python");
    C(table.contains("Rust", "sort_array"),
      "sort_array must be findable in Rust");
    P();
}

void t2(){
    T(lookup_by_language_and_name_returns_correct_record);
    ws::CrossLanguageSymbolTable table;
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");
    table.insert(node);

    auto record = table.lookup("Rust", "sort_array");

    C(record.found, "lookup must succeed");
    C(record.name == "sort_array",          "name must be sort_array");
    C(record.toComponent == "sort-core",    "toComponent must be sort-core");
    C(record.fromComponent == "data-gen",   "fromComponent must be data-gen");
    C(record.scipSymbol == "sort-core/sort_array.", "scipSymbol mismatch");
    P();
}

void t3(){
    T(lookup_by_scip_symbol_returns_all_appearances);
    ws::CrossLanguageSymbolTable table;
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");
    table.insert(node);

    auto appearances = table.lookupByScip("sort-core/sort_array.");

    C(appearances.size() == 2,
      "expected 2 appearances (Python+Rust), got " + std::to_string(appearances.size()));

    bool hasPython = false, hasRust = false;
    for (auto& a : appearances) {
        if (a.language == "Python") hasPython = true;
        if (a.language == "Rust")   hasRust   = true;
    }
    C(hasPython, "Python appearance not found");
    C(hasRust,   "Rust appearance not found");
    P();
}

void t4(){
    T(multiple_nodes_different_pairs_all_inserted);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array",   "data-gen",    "sort-core",  "Python", "Rust"));
    table.insert(makeNode("handle_req",   "http-server", "api-client", "Go",     "C++"));
    table.insert(makeNode("parse_result", "go-caller",   "rust-lib",   "Go",     "Rust"));

    C(table.contains("Python", "sort_array"),   "sort_array in Python");
    C(table.contains("Rust",   "sort_array"),   "sort_array in Rust");
    C(table.contains("Go",     "handle_req"),   "handle_req in Go");
    C(table.contains("C++",    "handle_req"),   "handle_req in C++");
    C(table.contains("Go",     "parse_result"), "parse_result in Go");
    C(table.contains("Rust",   "parse_result"), "parse_result in Rust");
    P();
}

void t5(){
    T(lookup_unknown_symbol_returns_not_found);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust"));

    auto record = table.lookup("Go", "nonexistent");
    C(!record.found, "lookup of unknown symbol must return found=false");

    auto appearances = table.lookupByScip("bogus/symbol.");
    C(appearances.empty(), "lookupByScip for unknown scipSymbol must return empty");
    P();
}

int main(){
    std::cout << "Step 1904: CrossLanguageSymbolTable\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
