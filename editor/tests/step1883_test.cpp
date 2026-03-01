// Step 1883 TDD: ASTFeatureExtractor — scalar feature extraction from AST subtrees
//
// Extracts: mutation ratio, recursion shape, concurrency primitives, I/O pattern, type complexity.
//
//  t1: flat assignment-heavy AST → high mutationRatio, FlatLoop shape
//  t2: recursive call tree → TreeRecursive shape, no concurrency
//  t3: channel-based AST → Channels concurrency, Blocking I/O
//  t4: async/await AST → Async I/O pattern, SimpleGenerics type complexity
//  t5: pure functional AST (no writes, tail calls) → low mutationRatio, TailRecursive

#include "ASTFeatureExtractor.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(flat_loop_high_mutation);
    auto ast = json::parse(R"({
        "type": "module",
        "children": [
            {"type": "for_stmt", "children": [
                {"type": "assign_stmt", "children": []},
                {"type": "assign_stmt", "children": []},
                {"type": "assign_stmt", "children": []}
            ]}
        ]
    })");
    auto feat = whetstone::ASTFeatureExtractor::extract(ast);
    C(feat.mutationRatio > 0.3f,
      "expected high mutationRatio for assign-heavy AST, got " + std::to_string(feat.mutationRatio));
    C(feat.recursionShape == AFF::RecursionShape::FlatLoop,
      "expected FlatLoop for for_stmt AST");
    P();
}

void t2(){
    T(tree_recursive_no_concurrency);
    auto ast = json::parse(R"({
        "type": "function_decl",
        "children": [
            {"type": "call_expr", "children": [
                {"type": "call_expr", "children": []}
            ]},
            {"type": "call_expr", "children": []}
        ]
    })");
    auto feat = whetstone::ASTFeatureExtractor::extract(ast);
    C(feat.recursionShape == AFF::RecursionShape::TreeRecursive,
      "expected TreeRecursive for multiple nested call_expr nodes");
    C(feat.concurrencyPrimitive == AFF::ConcurrencyPrimitive::None,
      "expected None concurrency for plain call tree");
    P();
}

void t3(){
    T(channel_concurrency_blocking_io);
    auto ast = json::parse(R"({
        "type": "function_decl",
        "children": [
            {"type": "chan_send", "children": []},
            {"type": "chan_recv", "children": []},
            {"type": "write_file", "children": []}
        ]
    })");
    auto feat = whetstone::ASTFeatureExtractor::extract(ast);
    C(feat.concurrencyPrimitive == AFF::ConcurrencyPrimitive::Channels,
      "expected Channels for chan_send/chan_recv nodes");
    C(feat.ioPattern == AFF::IOPattern::Blocking,
      "expected Blocking I/O for write_file node");
    P();
}

void t4(){
    T(async_io_simple_generics);
    auto ast = json::parse(R"({
        "type": "function_decl",
        "children": [
            {"type": "await_expr", "children": []},
            {"type": "async_call", "children": []},
            {"type": "type_param_decl", "children": []}
        ]
    })");
    auto feat = whetstone::ASTFeatureExtractor::extract(ast);
    C(feat.ioPattern == AFF::IOPattern::Async,
      "expected Async for await_expr/async_call nodes");
    C(feat.typeComplexity == AFF::TypeComplexity::SimpleGenerics,
      "expected SimpleGenerics for type_param_decl node");
    P();
}

void t5(){
    T(pure_functional_tail_recursive_low_mutation);
    // Tail recursive: call_expr as direct child of return_stmt, no loops, no writes
    auto ast = json::parse(R"({
        "type": "function_decl",
        "children": [
            {"type": "return_stmt", "children": [
                {"type": "call_expr", "children": []}
            ]}
        ]
    })");
    auto feat = whetstone::ASTFeatureExtractor::extract(ast);
    C(feat.mutationRatio < 0.1f,
      "expected low mutationRatio for pure functional AST, got " + std::to_string(feat.mutationRatio));
    C(feat.recursionShape == AFF::RecursionShape::TailRecursive,
      "expected TailRecursive for call_expr inside return_stmt");
    P();
}

int main(){
    std::cout << "Step 1883: ASTFeatureExtractor\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
