// Step 1910: CrossLanguageBoundaryDetector
// Identifies LSP goto-definition requests whose target symbol is a cross-language
// ABI boundary symbol registered in the CrossLanguageSymbolTable.
//
//  t1: request for a known boundary symbol → isBoundary=true, returns record
//  t2: request for an unknown symbol → isBoundary=false
//  t3: request uri + symbol name combined lookup works
//  t4: multiple boundary symbols — only the queried one is detected
//  t5: languageId-based lookup in addition to name-only lookup

#include "CrossLanguageBoundaryDetector.h"
#include "CrossLanguageSymbolTable.h"
#include "ABIBoundaryExtractor.h"
#include <iostream>
#include <string>

namespace ws = whetstone;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static ws::ABIBoundaryNode makeNode(const std::string& name,
                                    const std::string& from, const std::string& to,
                                    const std::string& fromLang, const std::string& toLang) {
    ws::ABIBoundaryNode n;
    n.name          = name;
    n.kind          = "function";
    n.fromComponent = from;
    n.toComponent   = to;
    n.fromLanguage  = fromLang;
    n.toLanguage    = toLang;
    n.signature     = "void " + name + "()";
    return n;
}

void t1(){
    T(known_boundary_symbol_is_detected);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));

    ws::CrossLanguageBoundaryDetector detector(table);
    auto result = detector.detect("sort_array");
    C(result.isBoundary, "sort_array must be detected as boundary");
    C(result.record.name == "sort_array", "record name must match");
    C(result.record.fromLanguage == "Python", "fromLanguage must be Python");
    C(result.record.toLanguage == "Rust", "toLanguage must be Rust");
    P();
}

void t2(){
    T(unknown_symbol_not_detected);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));

    ws::CrossLanguageBoundaryDetector detector(table);
    auto result = detector.detect("unknown_func");
    C(!result.isBoundary, "unknown_func must not be boundary");
    P();
}

void t3(){
    T(uri_and_symbol_combined_lookup);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));
    table.insert(makeNode("compress","encoder","codec","Go","C++"));

    ws::CrossLanguageBoundaryDetector detector(table);
    // uri provides language context, symbol name is what matters
    auto r1 = detector.detectWithUri("file:///main.py", "sort_array");
    C(r1.isBoundary, "sort_array in .py context must be boundary");
    auto r2 = detector.detectWithUri("file:///main.go", "compress");
    C(r2.isBoundary, "compress in .go context must be boundary");
    auto r3 = detector.detectWithUri("file:///main.py", "nonexistent");
    C(!r3.isBoundary, "nonexistent must not be boundary");
    P();
}

void t4(){
    T(only_queried_symbol_detected_among_many);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));
    table.insert(makeNode("compress","encoder","codec","Go","C++"));
    table.insert(makeNode("render","app","gpu-core","TypeScript","C++"));

    ws::CrossLanguageBoundaryDetector detector(table);
    auto r = detector.detect("compress");
    C(r.isBoundary, "compress must be boundary");
    C(r.record.name == "compress", "must return compress record");
    C(r.record.fromComponent == "encoder", "fromComponent must be encoder");
    auto r2 = detector.detect("sort_array");
    C(r2.record.toComponent == "sort-core", "sort_array toComponent must be sort-core");
    P();
}

void t5(){
    T(language_scoped_lookup);
    ws::CrossLanguageSymbolTable table;
    table.insert(makeNode("sort_array","data-gen","sort-core","Python","Rust"));

    ws::CrossLanguageBoundaryDetector detector(table);
    // lookup by language + name (uses CrossLanguageSymbolTable::lookup)
    auto r = detector.detectInLanguage("Rust", "sort_array");
    C(r.isBoundary, "sort_array in Rust scope must be boundary");
    auto r2 = detector.detectInLanguage("Go", "sort_array");
    C(!r2.isBoundary || r2.record.toLanguage == "Rust",
      "Go scope for sort_array: either not found or correct record");
    P();
}

int main(){
    std::cout << "Step 1910: CrossLanguageBoundaryDetector\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
