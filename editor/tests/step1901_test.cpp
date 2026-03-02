// Step 1901: whetstone_generate_ffi_glue MCP tool wiring
// Tests FFIGlueDispatcher logic directly (no MCPServer instantiation).
// Dispatcher selects the correct emitter based on fromLanguage/toLanguage on each boundary node.
//
//  t1: Python→Rust boundary dispatches to RustPython emitter (rustCode has extern/mod ffi)
//  t2: Go→C++ boundary dispatches to GoCpp emitter (goCode has CGo import)
//  t3: Go→Rust boundary dispatches to RustGo emitter (rustCode has no_mangle)
//  t4: TypeScript→C++ boundary dispatches to CppJS emitter (cppCode has napi_env)
//  t5: result JSON contains bindings array + c_header + dwarf_annotations

#include "FFIGlueDispatcher.h"
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
                                    const std::string& fromLang,
                                    const std::string& toLang)
{
    ws::ABIBoundaryNode n;
    n.name = name; n.kind = "function";
    n.fromComponent = "from"; n.toComponent = "to";
    n.fromLanguage = fromLang; n.toLanguage = toLang;
    n.signature = {{"name",name},{"kind","function"},{"exported",true},
                   {"returnType","i32"},{"params",json::array()}};
    return n;
}

void t1(){
    T(python_rust_dispatches_to_rust_python_emitter);
    auto node = makeNode("sort_array", "Python", "Rust");
    auto result = ws::FFIGlueDispatcher::dispatch(node);

    C(result.contains("rustCode") && !result["rustCode"].get<std::string>().empty(),
      "must have non-empty rustCode");
    C(result.contains("secondaryCode") && !result["secondaryCode"].get<std::string>().empty(),
      "must have non-empty secondaryCode (Python side)");
    std::string rust = result["rustCode"];
    C(rust.find("sort_array") != std::string::npos, "rustCode must contain sort_array");
    C(rust.find("extern") != std::string::npos || rust.find("mod ffi") != std::string::npos,
      "rustCode must have extern or mod ffi (RustPython pattern)");
    P();
}

void t2(){
    T(go_cpp_dispatches_to_go_cpp_emitter);
    auto node = makeNode("handle_request", "Go", "C++");
    auto result = ws::FFIGlueDispatcher::dispatch(node);

    C(result.contains("secondaryCode"), "must have secondaryCode (Go side)");
    std::string go = result["secondaryCode"];
    C(go.find("handle_request") != std::string::npos ||
      go.find("HandleRequest") != std::string::npos,
      "Go code must contain handle_request or HandleRequest");
    C(go.find("\"C\"") != std::string::npos || go.find("import") != std::string::npos,
      "Go code must have CGo import");
    P();
}

void t3(){
    T(go_rust_dispatches_to_rust_go_emitter);
    auto node = makeNode("compress", "Go", "Rust");
    auto result = ws::FFIGlueDispatcher::dispatch(node);

    C(result.contains("rustCode"), "must have rustCode");
    std::string rust = result["rustCode"];
    C(rust.find("no_mangle") != std::string::npos || rust.find("extern") != std::string::npos,
      "rustCode must have no_mangle or extern (RustGo pattern)");
    C(rust.find("compress") != std::string::npos, "rustCode must contain compress");
    P();
}

void t4(){
    T(typescript_cpp_dispatches_to_cpp_js_emitter);
    auto node = makeNode("get_version", "TypeScript", "C++");
    auto result = ws::FFIGlueDispatcher::dispatch(node);

    C(result.contains("rustCode"), "must have primary code field");
    std::string cpp = result["rustCode"]; // primary = cppCode
    C(cpp.find("napi_env") != std::string::npos || cpp.find("napi") != std::string::npos,
      "primary code must have napi (CppJS pattern)");
    P();
}

void t5(){
    T(generate_ffi_glue_result_has_required_top_level_fields);
    auto n1 = makeNode("fn_a", "Python", "Rust");
    auto n2 = makeNode("fn_b", "Go",     "C++");

    auto result = ws::FFIGlueDispatcher::generateAll({n1, n2}, "test_project");

    C(result.contains("bindings"),          "result must have bindings");
    C(result.contains("c_header"),          "result must have c_header");
    C(result.contains("dwarf_annotations"), "result must have dwarf_annotations");
    C(result["bindings"].is_array() && result["bindings"].size() == 2,
      "bindings must have 2 entries");
    C(result["dwarf_annotations"].is_array() && result["dwarf_annotations"].size() == 2,
      "dwarf_annotations must have 2 entries");
    std::string header = result["c_header"];
    C(header.find("#ifndef") != std::string::npos, "c_header must have include guard");
    P();
}

int main(){
    std::cout << "Step 1901: FFIGlueDispatcher / whetstone_generate_ffi_glue\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
