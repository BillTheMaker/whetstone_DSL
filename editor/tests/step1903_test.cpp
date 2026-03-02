// Step 1903: SCIPEmitter
// Emits a SCIP-format symbol index (as JSON) from ABIBoundaryNodes.
// One document per language, one symbol entry per boundary node visible in that language.
//
//  t1: single function node produces at least two documents (fromLanguage, toLanguage)
//  t2: each document has required SCIP fields: language, uri, symbols array
//  t3: symbol entry has name, kind, fromComponent, toComponent
//  t4: multiple nodes with same language land in the same document
//  t5: emit() output is a valid JSON object with "documents" array

#include "SCIPEmitter.h"
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
                                    const std::string& fromLang, const std::string& toLang,
                                    const std::string& kind = "function")
{
    ws::ABIBoundaryNode n;
    n.name = name; n.kind = kind;
    n.fromComponent = fromComp; n.toComponent = toComp;
    n.fromLanguage = fromLang;  n.toLanguage  = toLang;
    n.signature = {{"name",name},{"kind",kind},{"exported",true}};
    return n;
}

void t1(){
    T(single_node_produces_documents_for_both_languages);
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    auto index = ws::SCIPEmitter::emit({node});

    C(index.contains("documents") && index["documents"].is_array(),
      "index must have documents array");
    // Should have at least 2 documents: one for Python, one for Rust
    C(index["documents"].size() >= 2,
      "expected at least 2 documents (one per language), got " +
      std::to_string(index["documents"].size()));
    P();
}

void t2(){
    T(each_document_has_required_scip_fields);
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    auto index = ws::SCIPEmitter::emit({node});

    for (const auto& doc : index["documents"]) {
        C(doc.contains("language"),
          "document missing 'language' field");
        C(doc.contains("uri"),
          "document missing 'uri' field");
        C(doc.contains("symbols") && doc["symbols"].is_array(),
          "document missing 'symbols' array");
    }
    P();
}

void t3(){
    T(symbol_entry_has_name_kind_fromComponent_toComponent);
    auto node = makeNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    auto index = ws::SCIPEmitter::emit({node});

    bool found = false;
    for (const auto& doc : index["documents"]) {
        for (const auto& sym : doc["symbols"]) {
            if (sym.contains("name") && sym["name"] == "sort_array") {
                found = true;
                C(sym.contains("kind"),          "symbol missing 'kind'");
                C(sym.contains("fromComponent"), "symbol missing 'fromComponent'");
                C(sym.contains("toComponent"),   "symbol missing 'toComponent'");
            }
        }
    }
    C(found, "sort_array symbol not found in any document");
    P();
}

void t4(){
    T(multiple_nodes_same_language_land_in_same_document);
    // Two Rust-side exports from different interfaces
    auto n1 = makeNode("sort_array",  "data-gen", "sort-core", "Python", "Rust");
    auto n2 = makeNode("merge_slices","data-gen", "sort-core", "Python", "Rust");

    auto index = ws::SCIPEmitter::emit({n1, n2});

    // Count documents per language
    int rustDocCount = 0;
    for (const auto& doc : index["documents"]) {
        if (doc["language"] == "Rust") ++rustDocCount;
    }
    C(rustDocCount == 1,
      "both Rust nodes should share one Rust document, got " +
      std::to_string(rustDocCount));
    P();
}

void t5(){
    T(emit_returns_valid_json_with_documents_array);
    auto n1 = makeNode("fn_a", "alpha", "beta",  "Go",     "C++");
    auto n2 = makeNode("fn_b", "gamma", "delta", "Python", "Rust");

    auto index = ws::SCIPEmitter::emit({n1, n2});

    C(index.is_object(), "emit must return a JSON object");
    C(index.contains("documents"), "must have 'documents' key");
    C(index["documents"].is_array(), "documents must be an array");
    // Must have at least 4 language documents (Go, C++, Python, Rust)
    C(index["documents"].size() >= 4,
      "expected >=4 documents for 4 distinct languages");
    P();
}

int main(){
    std::cout << "Step 1903: SCIPEmitter\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
