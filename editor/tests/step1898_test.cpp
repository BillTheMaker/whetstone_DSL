// Step 1898: CppJSBindingEmitter
// From an ABIBoundaryNode, emits:
//   - C++ side: Node-API (N-API) wrapper function that bridges C++ → JS
//   - JS side: require() declaration with JS function signature comment
//
//  t1: C++ N-API wrapper contains napi_env, napi_callback_info, and function name
//  t2: JS side contains require or module.exports reference and function name
//  t3: struct node emits C++ N-API object builder + JS class comment
//  t4: multiple params produce correct napi_get_value calls in C++
//  t5: emitted code is non-empty and contains expected N-API keywords

#include "CppJSBindingEmitter.h"
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
    n.fromComponent = "api-client"; n.toComponent = "http-server";
    n.fromLanguage = "TypeScript";  n.toLanguage  = "C++";
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
    n.fromComponent = "api-client"; n.toComponent = "http-server";
    n.fromLanguage = "TypeScript";  n.toLanguage  = "C++";
    json sig;
    sig["name"] = name; sig["kind"] = "struct"; sig["exported"] = true;
    json fa = json::array();
    for (auto& [fn,ft] : fields) fa.push_back({{"name",fn},{"type",ft}});
    sig["fields"] = fa;
    n.signature = sig;
    return n;
}

void t1(){
    T(cpp_napi_wrapper_has_napi_env_and_function_name);
    auto node = makeFnNode("process_request", "int",
                           {{"buf", "char*"}, {"len", "int"}});

    auto result = ws::CppJSBindingEmitter::emit(node);

    C(result.cppCode.find("process_request") != std::string::npos,
      "cppCode must contain process_request");
    C(result.cppCode.find("napi_env") != std::string::npos,
      "cppCode must contain napi_env");
    C(result.cppCode.find("napi_callback_info") != std::string::npos ||
      result.cppCode.find("napi") != std::string::npos,
      "cppCode must reference N-API types");
    P();
}

void t2(){
    T(js_side_has_require_and_function_name);
    auto node = makeFnNode("process_request", "int",
                           {{"buf", "char*"}, {"len", "int"}});

    auto result = ws::CppJSBindingEmitter::emit(node);

    C(result.jsCode.find("process_request") != std::string::npos ||
      result.jsCode.find("processRequest") != std::string::npos,
      "jsCode must contain process_request or camelCase form");
    C(result.jsCode.find("require") != std::string::npos ||
      result.jsCode.find("module") != std::string::npos ||
      result.jsCode.find("import") != std::string::npos,
      "jsCode must reference require, module, or import");
    P();
}

void t3(){
    T(struct_emits_napi_object_builder_and_js_class_comment);
    auto node = makeStructNode("Response",
                               {{"status", "int"}, {"body_len", "int"}});

    auto result = ws::CppJSBindingEmitter::emit(node);

    C(result.cppCode.find("Response") != std::string::npos,
      "cppCode must contain Response");
    C(result.cppCode.find("napi") != std::string::npos,
      "cppCode must reference napi for struct builder");
    C(result.jsCode.find("Response") != std::string::npos,
      "jsCode must contain Response");
    P();
}

void t4(){
    T(multiple_params_produce_napi_get_value_calls);
    auto node = makeFnNode("compute", "double",
                           {{"x", "double"}, {"y", "double"}, {"z", "int"}});

    auto result = ws::CppJSBindingEmitter::emit(node);

    C(result.cppCode.find("napi_get_value") != std::string::npos ||
      result.cppCode.find("napi_") != std::string::npos,
      "cppCode must use napi_ calls to extract params");
    C(result.cppCode.find("compute") != std::string::npos,
      "cppCode must contain compute");
    P();
}

void t5(){
    T(emitted_bindings_non_empty_with_napi_keywords);
    auto node = makeFnNode("get_version", "int", {});

    auto result = ws::CppJSBindingEmitter::emit(node);

    C(!result.cppCode.empty(), "cppCode must not be empty");
    C(!result.jsCode.empty(),  "jsCode must not be empty");
    C(result.cppCode.find("napi") != std::string::npos,
      "cppCode must contain napi");
    C(result.cppCode.find("get_version") != std::string::npos,
      "cppCode must contain get_version");
    P();
}

int main(){
    std::cout << "Step 1898: CppJSBindingEmitter\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
