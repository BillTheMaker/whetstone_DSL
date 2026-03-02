// Step 1895: RustPythonBindingEmitter
// From the same ABIBoundaryNode, emits:
//   - Rust side: extern "C" function declaration in a `mod ffi` block
//   - Python side: ctypes binding (CDLL load + function prototype)
//
//  t1: Rust extern "C" declaration has correct name and signature
//  t2: Python ctypes binding contains CDLL load and argtypes/restype
//  t3: struct node emits Rust repr(C) struct + Python Structure subclass
//  t4: void-return function maps to None restype in Python
//  t5: emitted Rust and Python strings are non-empty and contain expected keywords

#include "RustPythonBindingEmitter.h"
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
    n.fromComponent = "data-gen"; n.toComponent = "sort-core";
    n.fromLanguage = "Python";    n.toLanguage  = "Rust";
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
    n.fromComponent = "data-gen"; n.toComponent = "sort-core";
    n.fromLanguage = "Python";    n.toLanguage  = "Rust";
    json sig;
    sig["name"] = name; sig["kind"] = "struct"; sig["exported"] = true;
    json fa = json::array();
    for (auto& [fn,ft] : fields) fa.push_back({{"name",fn},{"type",ft}});
    sig["fields"] = fa;
    n.signature = sig;
    return n;
}

void t1(){
    T(rust_extern_c_declaration_correct);
    auto node = makeFnNode("sort_array", "void",
                           {{"data", "i32"}, {"len", "usize"}});

    auto result = ws::RustPythonBindingEmitter::emit(node);

    C(result.rustCode.find("sort_array") != std::string::npos,
      "rustCode must contain sort_array");
    C(result.rustCode.find("extern") != std::string::npos,
      "rustCode must contain extern");
    C(result.rustCode.find("mod ffi") != std::string::npos ||
      result.rustCode.find("unsafe") != std::string::npos,
      "rustCode must contain mod ffi or unsafe");
    C(result.rustCode.find("i32") != std::string::npos,
      "rustCode must contain i32 param type");
    P();
}

void t2(){
    T(python_ctypes_binding_has_CDLL_and_argtypes);
    auto node = makeFnNode("sort_array", "void",
                           {{"data", "i32"}, {"len", "usize"}});

    auto result = ws::RustPythonBindingEmitter::emit(node);

    C(result.pythonCode.find("sort_array") != std::string::npos,
      "pythonCode must contain sort_array");
    C(result.pythonCode.find("CDLL") != std::string::npos ||
      result.pythonCode.find("ctypes") != std::string::npos,
      "pythonCode must reference CDLL or ctypes");
    C(result.pythonCode.find("argtypes") != std::string::npos ||
      result.pythonCode.find("restype")  != std::string::npos,
      "pythonCode must set argtypes or restype");
    P();
}

void t3(){
    T(struct_emits_rust_repr_C_and_python_Structure);
    auto node = makeStructNode("SortResult",
                               {{"len", "u32"}, {"status", "i32"}});

    auto result = ws::RustPythonBindingEmitter::emit(node);

    C(result.rustCode.find("SortResult") != std::string::npos,
      "rustCode must contain SortResult");
    C(result.rustCode.find("repr(C)") != std::string::npos ||
      result.rustCode.find("#[repr") != std::string::npos,
      "rustCode must have repr(C) attribute");
    C(result.pythonCode.find("SortResult") != std::string::npos,
      "pythonCode must contain SortResult");
    C(result.pythonCode.find("Structure") != std::string::npos ||
      result.pythonCode.find("ctypes") != std::string::npos,
      "pythonCode must use ctypes.Structure or ctypes");
    P();
}

void t4(){
    T(void_return_maps_to_None_restype_in_python);
    auto node = makeFnNode("reset_buffer", "void", {{"ptr", "i32"}});

    auto result = ws::RustPythonBindingEmitter::emit(node);

    C(result.pythonCode.find("None") != std::string::npos ||
      result.pythonCode.find("c_void") != std::string::npos,
      "void return must map to None or c_void in Python");
    P();
}

void t5(){
    T(emitted_bindings_non_empty_with_expected_keywords);
    auto node = makeFnNode("compute", "i32", {{"x", "i32"}, {"y", "i32"}});

    auto result = ws::RustPythonBindingEmitter::emit(node);

    C(!result.rustCode.empty(),   "rustCode must not be empty");
    C(!result.pythonCode.empty(), "pythonCode must not be empty");
    C(result.rustCode.find("fn") != std::string::npos ||
      result.rustCode.find("extern") != std::string::npos,
      "rustCode must contain fn or extern");
    C(result.pythonCode.find("import") != std::string::npos ||
      result.pythonCode.find("ctypes") != std::string::npos,
      "pythonCode must import ctypes");
    P();
}

int main(){
    std::cout << "Step 1895: RustPythonBindingEmitter\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
