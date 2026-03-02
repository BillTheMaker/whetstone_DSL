// Step 1897: Sprint 273 Integration
// Full pipeline: PolyglotProjectSpec → ABIBoundaryExtractor → CHeaderEmitter →
//   RustPythonBindingEmitter. Verified on poly-sort (data-gen/Python calls sort-core/Rust).
//
//  t1: poly-sort spec extracts correct boundary node from poly-sort AST
//  t2: C header generated from boundary node is syntactically correct
//  t3: Rust binding has extern "C" + mod ffi referencing sort_array
//  t4: Python binding has CDLL comment + argtypes + restype = None
//  t5: Sprint273IntegrationSummary struct reports correct counts and success

#include "ABIBoundaryExtractor.h"
#include "CHeaderEmitter.h"
#include "RustPythonBindingEmitter.h"
#include "PolySortProject.h"
#include "Sprint273IntegrationSummary.h"
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

// Poly-sort AST: sort-core (provider, toComponent) exports sort_array
// Interface in PolySortProject: from=data-gen, to=sort-core
static json makePolySortAST() {
    json ast;
    ast["components"]["sort-core"]["nodes"] = json::array({
        {{"name","sort_array"}, {"kind","function"}, {"exported",true},
         {"returnType","void"},
         {"params", json::array({
             {{"name","data"},{"type","i32*"}},
             {{"name","len"},{"type","usize"}}
         })}
        },
        {{"name","internal_merge"}, {"kind","function"}, {"exported",false}}
    });
    ast["components"]["data-gen"]["nodes"] = json::array({
        {{"name","generate"}, {"kind","function"}, {"exported",true}}
    });
    return ast;
}

void t1(){
    T(poly_sort_extracts_sort_array_boundary_node);
    auto spec = ws::PolySortProject::make();
    ws::PolyglotFitnessRouter::route(spec);
    auto ast = makePolySortAST();

    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);

    C(!nodes.empty(), "expected at least one boundary node");
    bool found = false;
    for (auto& n : nodes) {
        if (n.name == "sort_array") {
            found = true;
            C(n.fromComponent == "data-gen",
              "fromComponent should be data-gen, got: " + n.fromComponent);
            C(n.toComponent == "sort-core",
              "toComponent should be sort-core, got: " + n.toComponent);
            C(n.fromLanguage == "Python",
              "fromLanguage should be Python, got: " + n.fromLanguage);
            C(n.toLanguage == "Rust",
              "toLanguage should be Rust, got: " + n.toLanguage);
        }
    }
    C(found, "sort_array boundary node not found in poly-sort extraction");
    P();
}

void t2(){
    T(c_header_generated_correctly_from_boundary);
    auto spec = ws::PolySortProject::make();
    ws::PolyglotFitnessRouter::route(spec);
    auto ast  = makePolySortAST();
    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);

    C(!nodes.empty(), "no boundary nodes to emit header from");

    std::string header = ws::CHeaderEmitter::emit(nodes, "poly_sort");

    C(header.find("sort_array") != std::string::npos,
      "C header must contain sort_array");
    C(header.find("#ifndef") != std::string::npos,
      "C header must have include guard");
    C(header.find("extern \"C\"") != std::string::npos,
      "C header must have extern C block");
    C(header.back() == '\n', "C header must end with newline");
    P();
}

void t3(){
    T(rust_binding_has_extern_C_and_mod_ffi);
    auto spec = ws::PolySortProject::make();
    ws::PolyglotFitnessRouter::route(spec);
    auto ast   = makePolySortAST();
    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);
    C(!nodes.empty(), "no boundary nodes");

    // Find sort_array node
    ws::ABIBoundaryNode sortNode;
    for (auto& n : nodes) {
        if (n.name == "sort_array") { sortNode = n; break; }
    }
    C(sortNode.name == "sort_array", "sort_array node not found");

    auto binding = ws::RustPythonBindingEmitter::emit(sortNode);

    C(binding.rustCode.find("sort_array") != std::string::npos,
      "rustCode must contain sort_array");
    C(binding.rustCode.find("extern") != std::string::npos,
      "rustCode must have extern");
    C(binding.rustCode.find("mod ffi") != std::string::npos ||
      binding.rustCode.find("unsafe") != std::string::npos,
      "rustCode must have mod ffi or unsafe");
    P();
}

void t4(){
    T(python_binding_has_CDLL_argtypes_and_None_restype);
    auto spec = ws::PolySortProject::make();
    ws::PolyglotFitnessRouter::route(spec);
    auto ast   = makePolySortAST();
    auto nodes = ws::ABIBoundaryExtractor::extract(spec, ast);
    C(!nodes.empty(), "no boundary nodes");

    ws::ABIBoundaryNode sortNode;
    for (auto& n : nodes) {
        if (n.name == "sort_array") { sortNode = n; break; }
    }
    C(sortNode.name == "sort_array", "sort_array node not found");

    auto binding = ws::RustPythonBindingEmitter::emit(sortNode);

    C(binding.pythonCode.find("sort_array") != std::string::npos,
      "pythonCode must contain sort_array");
    C(binding.pythonCode.find("CDLL") != std::string::npos ||
      binding.pythonCode.find("ctypes") != std::string::npos,
      "pythonCode must reference CDLL or ctypes");
    C(binding.pythonCode.find("argtypes") != std::string::npos,
      "pythonCode must set argtypes");
    // sort_array returns void → restype = None
    C(binding.pythonCode.find("None") != std::string::npos,
      "pythonCode must have restype = None for void function");
    P();
}

void t5(){
    T(sprint273_integration_summary_correct);
    ws::Sprint273IntegrationSummary s;
    C(s.stepsCompleted == 5, "expected 5 steps completed");
    C(s.bindingPairsImplemented == 2,
      "expected 2 binding pairs (RustPython, GoCpp)");
    C(s.success, "success must be true");
    C(s.sprintName() == "Sprint 273: PolyglotFFIGlueGenerator",
      "sprintName mismatch: " + s.sprintName());
    P();
}

int main(){
    std::cout << "Step 1897: Sprint 273 Integration\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
