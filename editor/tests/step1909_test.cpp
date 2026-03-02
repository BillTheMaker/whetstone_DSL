// Step 1909: LanguageServerRouter
// Routes LSP requests to the correct per-language server stub based on
// file URI extension or explicit languageId.
//
//  t1: .py URI routes to Python server
//  t2: .rs URI routes to Rust server
//  t3: .go URI routes to Go server
//  t4: .ts/.js URI routes to TypeScript server
//  t5: unknown extension routes to "unknown", .cpp/.h routes to Cpp

#include "LanguageServerRouter.h"
#include <iostream>
#include <string>

namespace ws = whetstone;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(py_uri_routes_to_python);
    ws::LanguageServerRouter router;
    C(router.routeByUri("file:///src/sort.py") == "Python",
      "expected Python for .py");
    C(router.routeByUri("file:///a/b/c.py")   == "Python",
      "expected Python for any .py");
    P();
}

void t2(){
    T(rs_uri_routes_to_rust);
    ws::LanguageServerRouter router;
    C(router.routeByUri("file:///src/lib.rs") == "Rust",
      "expected Rust for .rs");
    P();
}

void t3(){
    T(go_uri_routes_to_go);
    ws::LanguageServerRouter router;
    C(router.routeByUri("file:///main.go") == "Go",
      "expected Go for .go");
    P();
}

void t4(){
    T(ts_and_js_uri_routes_to_typescript);
    ws::LanguageServerRouter router;
    C(router.routeByUri("file:///app.ts") == "TypeScript",
      "expected TypeScript for .ts");
    C(router.routeByUri("file:///app.js") == "TypeScript",
      "expected TypeScript for .js");
    P();
}

void t5(){
    T(cpp_h_routes_to_cpp_unknown_extension_routes_unknown);
    ws::LanguageServerRouter router;
    C(router.routeByUri("file:///engine.cpp") == "C++",
      "expected C++ for .cpp");
    C(router.routeByUri("file:///engine.h")   == "C++",
      "expected C++ for .h");
    C(router.routeByUri("file:///data.json")  == "unknown",
      "expected unknown for .json");
    // languageId override
    C(router.routeByLanguageId("python") == "Python",
      "expected Python for languageId=python");
    C(router.routeByLanguageId("rust")   == "Rust",
      "expected Rust for languageId=rust");
    P();
}

int main(){
    std::cout << "Step 1909: LanguageServerRouter\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
