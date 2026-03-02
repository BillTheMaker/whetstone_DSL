// Step 1912: Sprint 276 Integration
// End-to-end: poly-sort project — goto-definition from a Python call site
// lands on the Rust sort-core provider via LSP proxy + cross-language resolver.
//
//  t1: PolySortProject symbols load into symbol table
//  t2: LSP textDocument/definition request routes through proxy
//  t3: CrossLanguageBoundaryDetector identifies sort_array as boundary
//  t4: CrossLanguageDefinitionResolver returns Rust provider location
//  t5: Sprint276IntegrationSummary reports complete

#include "LSPProxyServer.h"
#include "LanguageServerRouter.h"
#include "CrossLanguageBoundaryDetector.h"
#include "CrossLanguageDefinitionResolver.h"
#include "CrossLanguageSymbolTable.h"
#include "ABIBoundaryExtractor.h"
#include "Sprint276IntegrationSummary.h"
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

// Build a PolySortProject symbol table
static ws::CrossLanguageSymbolTable buildPolySortTable() {
    ws::CrossLanguageSymbolTable table;
    ws::ABIBoundaryNode n;
    n.name          = "sort_array";
    n.kind          = "function";
    n.fromComponent = "data-gen";
    n.toComponent   = "sort-core";
    n.fromLanguage  = "Python";
    n.toLanguage    = "Rust";
    n.signature     = "void sort_array(int* data, int len)";
    table.insert(n);

    ws::ABIBoundaryNode n2;
    n2.name          = "benchmark_sort";
    n2.kind          = "function";
    n2.fromComponent = "data-gen";
    n2.toComponent   = "sort-core";
    n2.fromLanguage  = "Python";
    n2.toLanguage    = "Rust";
    n2.signature     = "double benchmark_sort(int iterations)";
    table.insert(n2);
    return table;
}

void t1(){
    T(polysort_symbols_load_into_table);
    auto table = buildPolySortTable();
    C(table.contains("Python", "sort_array"),    "Python must contain sort_array");
    C(table.contains("Rust",   "sort_array"),    "Rust must contain sort_array");
    C(table.contains("Python", "benchmark_sort"),"Python must contain benchmark_sort");
    P();
}

void t2(){
    T(lsp_definition_request_routed_through_proxy);
    ws::LSPProxyServer server;
    // Register a cross-lang handler for textDocument/definition
    auto table = buildPolySortTable();
    ws::CrossLanguageDefinitionResolver resolver(table);

    server.registerHandler("textDocument/definition",
        [&resolver](const json& params) -> json {
            std::string uri    = params.value("textDocument", json::object())
                                        .value("uri", "");
            std::string symbol = params.value("symbol", "sort_array");
            auto loc = resolver.resolveFromUri(uri, symbol);
            if (!loc.found) return nullptr;
            return {{"uri", "scip://" + loc.providerComponent},
                    {"provider", loc.providerComponent},
                    {"language", loc.providerLanguage},
                    {"scipSymbol", loc.scipSymbol}};
        });

    json req = {{"jsonrpc","2.0"},{"id",10},{"method","textDocument/definition"},
                {"params",{{"textDocument",{{"uri","file:///src/main.py"}}},
                           {"symbol","sort_array"}}}};
    auto resp = server.handle(req);
    C(resp.contains("result"), "must have result");
    C(!resp["result"].is_null(), "result must not be null");
    C(resp["result"]["language"] == "Rust", "provider language must be Rust");
    P();
}

void t3(){
    T(boundary_detector_identifies_sort_array);
    auto table = buildPolySortTable();
    ws::CrossLanguageBoundaryDetector detector(table);
    auto r = detector.detectWithUri("file:///src/main.py", "sort_array");
    C(r.isBoundary, "sort_array must be boundary");
    C(r.record.toComponent == "sort-core", "provider must be sort-core");
    P();
}

void t4(){
    T(resolver_returns_rust_provider_location);
    auto table = buildPolySortTable();
    ws::CrossLanguageDefinitionResolver resolver(table);
    auto loc = resolver.resolveFromUri("file:///src/main.py", "sort_array");
    C(loc.found,                         "must resolve");
    C(loc.providerComponent == "sort-core","provider component");
    C(loc.providerLanguage  == "Rust",    "provider language");
    C(loc.callerLanguage    == "Python",  "caller language");
    C(loc.scipSymbol == "sort-core/sort_array.", "scipSymbol");
    P();
}

void t5(){
    T(sprint276_integration_summary_reports_complete);
    ws::Sprint276IntegrationSummary summary;
    C(summary.stepsCompleted == 5, "5 steps");
    C(summary.success,             "success");
    C(summary.sprintName().find("276") != std::string::npos, "sprint name");
    P();
}

int main(){
    std::cout << "Step 1912: Sprint 276 Integration\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
