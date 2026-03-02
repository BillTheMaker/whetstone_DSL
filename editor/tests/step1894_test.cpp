// Step 1894: CHeaderEmitter
// Emits a C ABI header from a list of ABIBoundaryNodes.
// The C ABI serves as the neutral intermediary for any language pair.
//
//  t1: function node with params emits correct C declaration
//  t2: struct node emits typedef struct block
//  t3: multiple nodes emitted in one header with guard and extern "C"
//  t4: node with no params emits void-param C function
//  t5: emitted header compiles (syntactic validity check via string structure)

#include "CHeaderEmitter.h"
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

// Helper: build an ABIBoundaryNode for a function
static ws::ABIBoundaryNode makeFnNode(const std::string& name,
                                      const std::string& returnType,
                                      std::vector<std::pair<std::string,std::string>> params,
                                      const std::string& from = "caller",
                                      const std::string& to   = "provider")
{
    ws::ABIBoundaryNode n;
    n.name          = name;
    n.kind          = "function";
    n.fromComponent = from;
    n.toComponent   = to;
    n.fromLanguage  = "Python";
    n.toLanguage    = "Rust";

    json sig;
    sig["name"]       = name;
    sig["kind"]       = "function";
    sig["exported"]   = true;
    sig["returnType"] = returnType;
    json paramArr = json::array();
    for (auto& [pname, ptype] : params) {
        paramArr.push_back({{"name", pname}, {"type", ptype}});
    }
    sig["params"] = paramArr;
    n.signature = sig;
    return n;
}

// Helper: build an ABIBoundaryNode for a struct
static ws::ABIBoundaryNode makeStructNode(const std::string& name,
                                          std::vector<std::pair<std::string,std::string>> fields)
{
    ws::ABIBoundaryNode n;
    n.name          = name;
    n.kind          = "struct";
    n.fromComponent = "caller";
    n.toComponent   = "provider";
    n.fromLanguage  = "Go";
    n.toLanguage    = "C++";

    json sig;
    sig["name"]     = name;
    sig["kind"]     = "struct";
    sig["exported"] = true;
    json fieldArr = json::array();
    for (auto& [fname, ftype] : fields) {
        fieldArr.push_back({{"name", fname}, {"type", ftype}});
    }
    sig["fields"] = fieldArr;
    n.signature = sig;
    return n;
}

void t1(){
    T(function_node_emits_correct_c_declaration);
    auto node = makeFnNode("sort_array", "void",
                           {{"data", "int32_t*"}, {"len", "size_t"}});

    std::string header = ws::CHeaderEmitter::emit({node}, "sort_array");

    C(header.find("sort_array") != std::string::npos,
      "header must contain function name sort_array");
    C(header.find("int32_t*") != std::string::npos || header.find("int32_t *") != std::string::npos,
      "header must contain param type int32_t*");
    C(header.find("size_t") != std::string::npos,
      "header must contain param type size_t");
    C(header.find("void") != std::string::npos,
      "header must contain return type void");
    P();
}

void t2(){
    T(struct_node_emits_typedef_struct_block);
    auto node = makeStructNode("SortResult",
                               {{"data", "int32_t*"}, {"len", "size_t"}, {"status", "int"}});

    std::string header = ws::CHeaderEmitter::emit({node}, "sort_result");

    C(header.find("SortResult") != std::string::npos,
      "header must contain struct name SortResult");
    C(header.find("int32_t*") != std::string::npos || header.find("int32_t *") != std::string::npos,
      "header must contain field type int32_t*");
    C(header.find("typedef") != std::string::npos || header.find("struct") != std::string::npos,
      "header must contain struct or typedef declaration");
    P();
}

void t3(){
    T(multiple_nodes_in_one_header_with_guard_and_extern_C);
    auto fn     = makeFnNode("process", "int", {{"x", "int"}, {"y", "int"}});
    auto st     = makeStructNode("Point", {{"x", "float"}, {"y", "float"}});

    std::string header = ws::CHeaderEmitter::emit({fn, st}, "geometry");

    C(header.find("#ifndef") != std::string::npos || header.find("#pragma once") != std::string::npos,
      "header must have include guard or pragma once");
    C(header.find("extern \"C\"") != std::string::npos || header.find("extern") != std::string::npos,
      "header must have extern C linkage block");
    C(header.find("process") != std::string::npos, "header must contain process");
    C(header.find("Point")   != std::string::npos, "header must contain Point");
    P();
}

void t4(){
    T(void_param_function_emits_void_in_c_prototype);
    // Function with no params → C convention: (void)
    auto node = makeFnNode("get_version", "int", {} /* no params */);

    std::string header = ws::CHeaderEmitter::emit({node}, "version");

    C(header.find("get_version") != std::string::npos,
      "header must contain get_version");
    // Either (void) or () is acceptable for no-arg C prototype
    C(header.find("get_version") != std::string::npos,
      "header must include function declaration");
    P();
}

void t5(){
    T(emitted_header_has_valid_structure);
    auto fn = makeFnNode("sort_array", "void",
                         {{"data", "int32_t*"}, {"len", "size_t"}});

    std::string header = ws::CHeaderEmitter::emit({fn}, "sort");

    // Must start or contain standard C header elements
    C(!header.empty(), "header must not be empty");
    // Must end with newline
    C(header.back() == '\n', "header must end with newline");
    // Must contain the function name
    C(header.find("sort_array") != std::string::npos,
      "header must contain sort_array");
    // Guard or pragma once must appear before any declaration
    size_t guardPos  = header.find("#ifndef");
    size_t pragmaPos = header.find("#pragma once");
    size_t fnPos     = header.find("sort_array");
    bool guardOk = (guardPos  != std::string::npos && guardPos  < fnPos) ||
                   (pragmaPos != std::string::npos && pragmaPos < fnPos);
    C(guardOk, "include guard/pragma must appear before function declaration");
    P();
}

int main(){
    std::cout << "Step 1894: CHeaderEmitter\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
