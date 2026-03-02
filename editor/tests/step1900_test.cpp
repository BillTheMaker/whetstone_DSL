// Step 1900: DWARFBoundaryAnnotator
// Annotates a generated DWARF stub with cross-language AST node IDs.
// Needed by the DAP orchestrator to correlate debug events across language boundaries.
// Output: JSON annotation records (not actual DWARF binary — string-level annotation).
//
//  t1: function node produces annotation with name, fromComponent, toComponent, astNodeId
//  t2: annotation includes source language pair
//  t3: multiple nodes produce one annotation each
//  t4: struct node annotation has kind=struct
//  t5: annotate() returns valid JSON array

#include "DWARFBoundaryAnnotator.h"
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

static ws::ABIBoundaryNode makeFnNode(const std::string& name,
                                      const std::string& from, const std::string& to,
                                      const std::string& fromLang, const std::string& toLang)
{
    ws::ABIBoundaryNode n;
    n.name = name; n.kind = "function";
    n.fromComponent = from; n.toComponent = to;
    n.fromLanguage = fromLang; n.toLanguage = toLang;
    n.signature = {{"name", name}, {"kind", "function"}, {"exported", true}};
    return n;
}

static ws::ABIBoundaryNode makeStructNode(const std::string& name,
                                          const std::string& from, const std::string& to)
{
    ws::ABIBoundaryNode n;
    n.name = name; n.kind = "struct";
    n.fromComponent = from; n.toComponent = to;
    n.fromLanguage = "Go"; n.toLanguage = "C++";
    n.signature = {{"name", name}, {"kind", "struct"}, {"exported", true}};
    return n;
}

void t1(){
    T(function_annotation_has_required_fields);
    auto node = makeFnNode("sort_array", "data-gen", "sort-core", "Python", "Rust");

    auto annotations = ws::DWARFBoundaryAnnotator::annotate({node});

    C(annotations.is_array(), "result must be a JSON array");
    C(!annotations.empty(), "must have at least one annotation");

    auto& ann = annotations[0];
    C(ann.contains("name") && ann["name"] == "sort_array",
      "annotation must have name=sort_array");
    C(ann.contains("fromComponent") && ann["fromComponent"] == "data-gen",
      "annotation must have fromComponent=data-gen");
    C(ann.contains("toComponent") && ann["toComponent"] == "sort-core",
      "annotation must have toComponent=sort-core");
    C(ann.contains("astNodeId") && ann["astNodeId"].is_string() && !ann["astNodeId"].get<std::string>().empty(),
      "annotation must have non-empty astNodeId");
    P();
}

void t2(){
    T(annotation_includes_language_pair);
    auto node = makeFnNode("process", "caller", "provider", "Go", "C++");

    auto annotations = ws::DWARFBoundaryAnnotator::annotate({node});

    C(!annotations.empty(), "must have annotation");
    auto& ann = annotations[0];
    C(ann.contains("fromLanguage") && ann["fromLanguage"] == "Go",
      "annotation must have fromLanguage=Go");
    C(ann.contains("toLanguage") && ann["toLanguage"] == "C++",
      "annotation must have toLanguage=C++");
    P();
}

void t3(){
    T(multiple_nodes_produce_one_annotation_each);
    auto n1 = makeFnNode("fn_a", "alpha", "beta",  "Rust",   "Python");
    auto n2 = makeFnNode("fn_b", "alpha", "gamma", "Rust",   "Go");
    auto n3 = makeFnNode("fn_c", "beta",  "delta", "Python", "TypeScript");

    auto annotations = ws::DWARFBoundaryAnnotator::annotate({n1, n2, n3});

    C(annotations.size() == 3, "expected 3 annotations, got " +
      std::to_string(annotations.size()));
    P();
}

void t4(){
    T(struct_annotation_has_kind_struct);
    auto node = makeStructNode("RequestHeader", "http-server", "api-client");

    auto annotations = ws::DWARFBoundaryAnnotator::annotate({node});

    C(!annotations.empty(), "must have annotation");
    auto& ann = annotations[0];
    C(ann.contains("kind") && ann["kind"] == "struct",
      "struct annotation must have kind=struct");
    C(ann["name"] == "RequestHeader",
      "name must be RequestHeader");
    P();
}

void t5(){
    T(annotate_returns_valid_json_array);
    auto node = makeFnNode("compute", "a", "b", "TypeScript", "C++");

    json result = ws::DWARFBoundaryAnnotator::annotate({node});

    C(result.is_array(), "result must be a JSON array");
    C(result.size() == 1, "expected 1 element");
    // astNodeId must be deterministic for same input
    std::string id1 = result[0]["astNodeId"];
    json result2 = ws::DWARFBoundaryAnnotator::annotate({node});
    std::string id2 = result2[0]["astNodeId"];
    C(id1 == id2, "astNodeId must be deterministic for same input");
    P();
}

int main(){
    std::cout << "Step 1900: DWARFBoundaryAnnotator\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
