// Step 1896: GoCppBindingEmitter
// From the same ABIBoundaryNode, emits:
//   - C++ side: extern "C" function implementation stub
//   - Go side: CGo `import "C"` bridge + Go wrapper function
//
//  t1: C++ extern "C" declaration contains correct signature
//  t2: Go side contains import "C" and wrapper function
//  t3: struct node emits C++ struct + Go CGo struct
//  t4: multiple params emitted correctly in both sides
//  t5: emitted bindings are non-empty and syntactically plausible

#include "GoCppBindingEmitter.h"
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
    n.fromComponent = "http-server"; n.toComponent = "api-client";
    n.fromLanguage = "Go";           n.toLanguage  = "C++";
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
    n.fromComponent = "http-server"; n.toComponent = "api-client";
    n.fromLanguage = "Go";           n.toLanguage  = "C++";
    json sig;
    sig["name"] = name; sig["kind"] = "struct"; sig["exported"] = true;
    json fa = json::array();
    for (auto& [fn,ft] : fields) fa.push_back({{"name",fn},{"type",ft}});
    sig["fields"] = fa;
    n.signature = sig;
    return n;
}

void t1(){
    T(cpp_extern_C_declaration_correct);
    auto node = makeFnNode("handle_request", "int",
                           {{"buf", "char*"}, {"len", "int"}});

    auto result = ws::GoCppBindingEmitter::emit(node);

    C(result.cppCode.find("handle_request") != std::string::npos,
      "cppCode must contain handle_request");
    C(result.cppCode.find("extern") != std::string::npos,
      "cppCode must contain extern");
    C(result.cppCode.find("char") != std::string::npos ||
      result.cppCode.find("buf")  != std::string::npos,
      "cppCode must contain param type or name");
    P();
}

void t2(){
    T(go_side_has_import_C_and_wrapper);
    auto node = makeFnNode("handle_request", "int",
                           {{"buf", "char*"}, {"len", "int"}});

    auto result = ws::GoCppBindingEmitter::emit(node);

    C(result.goCode.find("\"C\"") != std::string::npos ||
      result.goCode.find("import") != std::string::npos,
      "goCode must import C or use import");
    C(result.goCode.find("handle_request") != std::string::npos ||
      result.goCode.find("HandleRequest") != std::string::npos,
      "goCode must contain handle_request or Go-cased wrapper");
    C(result.goCode.find("func") != std::string::npos,
      "goCode must define a Go func");
    P();
}

void t3(){
    T(struct_emits_cpp_struct_and_go_cgo_struct);
    auto node = makeStructNode("RequestHeader",
                               {{"method", "int"}, {"url_len", "int"}});

    auto result = ws::GoCppBindingEmitter::emit(node);

    C(result.cppCode.find("RequestHeader") != std::string::npos,
      "cppCode must contain RequestHeader");
    C(result.cppCode.find("struct") != std::string::npos,
      "cppCode must use struct");
    C(result.goCode.find("RequestHeader") != std::string::npos,
      "goCode must contain RequestHeader");
    C(result.goCode.find("C.") != std::string::npos ||
      result.goCode.find("struct") != std::string::npos ||
      result.goCode.find("type") != std::string::npos,
      "goCode must reference CGo or define Go type");
    P();
}

void t4(){
    T(multiple_params_emitted_correctly_in_both_sides);
    auto node = makeFnNode("process",  "int",
                           {{"a", "int"}, {"b", "int"}, {"c", "float"}});

    auto result = ws::GoCppBindingEmitter::emit(node);

    C(result.cppCode.find("a") != std::string::npos ||
      result.cppCode.find("int") != std::string::npos,
      "cppCode must contain params");
    C(result.goCode.find("process") != std::string::npos ||
      result.goCode.find("Process") != std::string::npos,
      "goCode must contain process or Process");
    P();
}

void t5(){
    T(emitted_bindings_non_empty_and_plausible);
    auto node = makeFnNode("get_status", "int", {});

    auto result = ws::GoCppBindingEmitter::emit(node);

    C(!result.cppCode.empty(), "cppCode must not be empty");
    C(!result.goCode.empty(),  "goCode must not be empty");
    C(result.cppCode.find("get_status") != std::string::npos,
      "cppCode must contain get_status");
    C(result.goCode.find("get_status") != std::string::npos ||
      result.goCode.find("GetStatus")  != std::string::npos,
      "goCode must reference get_status or GetStatus");
    P();
}

int main(){
    std::cout << "Step 1896: GoCppBindingEmitter\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
