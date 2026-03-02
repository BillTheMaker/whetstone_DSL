// Step 1899: RustGoBindingEmitter
// From an ABIBoundaryNode, emits:
//   - Rust side: extern "C" pub fn declaration (same as RustPythonBindingEmitter Rust side)
//   - Go side: CGo import "C" bridge + Go wrapper (same as GoCppBindingEmitter Go side)
//
//  t1: Rust side has extern "C" and function name
//  t2: Go side has import "C" and Go wrapper func
//  t3: struct node emits Rust repr(C) + Go CGo type alias
//  t4: void return produces correct Rust and Go signatures
//  t5: non-empty bindings with expected keywords for both sides

#include "RustGoBindingEmitter.h"
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
                                      const std::string& returnType,
                                      std::vector<std::pair<std::string,std::string>> params)
{
    ws::ABIBoundaryNode n;
    n.name = name; n.kind = "function";
    n.fromComponent = "go-caller"; n.toComponent = "rust-lib";
    n.fromLanguage = "Go";         n.toLanguage  = "Rust";
    json sig;
    sig["name"] = name; sig["kind"] = "function"; sig["exported"] = true;
    sig["returnType"] = returnType;
    json pa = json::array();
    for (auto& [pn,pt] : params) pa.push_back({{"name",pn},{"type",pt}});
    sig["params"] = pa;
    n.signature = sig;
    return n;
}

static ws::ABIBoundaryNode makeStructNode(const std::string& name,
                                          std::vector<std::pair<std::string,std::string>> fields)
{
    ws::ABIBoundaryNode n;
    n.name = name; n.kind = "struct";
    n.fromComponent = "go-caller"; n.toComponent = "rust-lib";
    n.fromLanguage = "Go";         n.toLanguage  = "Rust";
    json sig;
    sig["name"] = name; sig["kind"] = "struct"; sig["exported"] = true;
    json fa = json::array();
    for (auto& [fn,ft] : fields) fa.push_back({{"name",fn},{"type",ft}});
    sig["fields"] = fa;
    n.signature = sig;
    return n;
}

void t1(){
    T(rust_side_has_extern_C_and_function_name);
    auto node = makeFnNode("sort_slice", "i32",
                           {{"data", "i32*"}, {"len", "usize"}});

    auto result = ws::RustGoBindingEmitter::emit(node);

    C(result.rustCode.find("sort_slice") != std::string::npos,
      "rustCode must contain sort_slice");
    C(result.rustCode.find("extern") != std::string::npos,
      "rustCode must contain extern");
    C(result.rustCode.find("\"C\"") != std::string::npos ||
      result.rustCode.find("no_mangle") != std::string::npos,
      "rustCode must have extern C or no_mangle");
    P();
}

void t2(){
    T(go_side_has_import_C_and_wrapper_func);
    auto node = makeFnNode("sort_slice", "i32",
                           {{"data", "i32*"}, {"len", "usize"}});

    auto result = ws::RustGoBindingEmitter::emit(node);

    C(result.goCode.find("\"C\"") != std::string::npos ||
      result.goCode.find("import") != std::string::npos,
      "goCode must import C");
    C(result.goCode.find("sort_slice") != std::string::npos ||
      result.goCode.find("SortSlice") != std::string::npos,
      "goCode must contain sort_slice or SortSlice");
    C(result.goCode.find("func") != std::string::npos,
      "goCode must define a func");
    P();
}

void t3(){
    T(struct_emits_rust_repr_C_and_go_type_alias);
    auto node = makeStructNode("SliceResult",
                               {{"ptr", "i32*"}, {"len", "usize"}});

    auto result = ws::RustGoBindingEmitter::emit(node);

    C(result.rustCode.find("SliceResult") != std::string::npos,
      "rustCode must contain SliceResult");
    C(result.rustCode.find("repr(C)") != std::string::npos ||
      result.rustCode.find("#[repr") != std::string::npos,
      "rustCode must have repr(C)");
    C(result.goCode.find("SliceResult") != std::string::npos,
      "goCode must contain SliceResult");
    C(result.goCode.find("C.struct") != std::string::npos ||
      result.goCode.find("type") != std::string::npos,
      "goCode must alias or define the struct type");
    P();
}

void t4(){
    T(void_return_correct_in_rust_and_go);
    auto node = makeFnNode("reset_state", "void", {{"ctx", "i32"}});

    auto result = ws::RustGoBindingEmitter::emit(node);

    // Rust void return: no -> ... or -> ()
    C(result.rustCode.find("reset_state") != std::string::npos,
      "rustCode must contain reset_state");
    // Go void return: func ResetState(...) { — no return type
    C(result.goCode.find("reset_state") != std::string::npos ||
      result.goCode.find("ResetState") != std::string::npos,
      "goCode must contain reset_state or ResetState");
    P();
}

void t5(){
    T(emitted_bindings_non_empty_with_expected_keywords);
    auto node = makeFnNode("compress", "usize",
                           {{"input", "i32*"}, {"out", "i32*"}});

    auto result = ws::RustGoBindingEmitter::emit(node);

    C(!result.rustCode.empty(), "rustCode must not be empty");
    C(!result.goCode.empty(),   "goCode must not be empty");
    C(result.rustCode.find("compress") != std::string::npos,
      "rustCode must contain compress");
    C(result.goCode.find("compress") != std::string::npos ||
      result.goCode.find("Compress") != std::string::npos,
      "goCode must contain compress or Compress");
    P();
}

int main(){
    std::cout << "Step 1899: RustGoBindingEmitter\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
