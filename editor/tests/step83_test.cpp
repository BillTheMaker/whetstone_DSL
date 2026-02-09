// Step 83 TDD Test: Additional tree-sitter grammars
//
// Tests:
// 1. SyntaxHighlighter returns spans for new languages
// 2. EditorMode can switch to new languages

#include <cassert>
#include <iostream>
#include "SyntaxHighlighter.h"
#include "EditorMode.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: SyntaxHighlighter spans ---
    {
        const char* js = "function add(a,b){ return a+b; }";
        const char* ts = "function add(a: number, b: number): number { return a + b; }";
        const char* java = "class A { int add(int a, int b){ return a+b; } }";
        const char* rust = "fn add(a:i32,b:i32)->i32{ a+b }";
        const char* go = "func add(a int, b int) int { return a+b }";

        auto jsSpans = SyntaxHighlighter::highlight(js, "javascript");
        auto tsSpans = SyntaxHighlighter::highlight(ts, "typescript");
        auto javaSpans = SyntaxHighlighter::highlight(java, "java");
        auto rustSpans = SyntaxHighlighter::highlight(rust, "rust");
        auto goSpans = SyntaxHighlighter::highlight(go, "go");

        assert(!jsSpans.empty());
        assert(!tsSpans.empty());
        assert(!javaSpans.empty());
        assert(!rustSpans.empty());
        assert(!goSpans.empty());

        std::cout << "Test 1 PASS: Highlight spans for new languages" << std::endl;
        ++passed;
    }

    // --- Test 2: EditorMode switches ---
    {
        EditorMode mode("javascript");
        assert(mode.getLanguage() == "javascript");
        mode.setLanguage("rust");
        assert(mode.getLanguage() == "rust");
        std::cout << "Test 2 PASS: EditorMode supports new languages" << std::endl;
        ++passed;
    }

    std::cout << "\n=== Step 83 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
