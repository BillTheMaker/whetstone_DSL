// Step 311: C# Parser Tests (12 tests)
// Verifies CSharpParser parses C# source into Whetstone AST:
// methods, async methods → AsyncFunction, class, interface, using/namespace skipped.

#include "ast/CSharpParser.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/ClassDeclaration.h"
#include "ast/AsyncNodes.h"
#include <iostream>
#include <string>
#include <memory>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. Parse a simple method
void test_parse_simple_method() {
    TEST(parse_simple_method);
    auto mod = CSharpParser::parseCSharp(R"(
public void DoWork() {
    Console.WriteLine("Hello");
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function, got " + std::to_string(fns.size()));
    CHECK(fns[0]->conceptType == "Function", "expected Function, got " + fns[0]->conceptType);
    auto* fn = static_cast<const Function*>(fns[0]);
    CHECK(fn->name == "DoWork", "expected name 'DoWork', got '" + fn->name + "'");
    PASS();
}

// 2. Parse async method → AsyncFunction
void test_parse_async_method() {
    TEST(parse_async_method);
    auto mod = CSharpParser::parseCSharp(R"(
public async Task FetchData() {
    var result = await httpClient.GetAsync(url);
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    CHECK(fns[0]->conceptType == "AsyncFunction", "expected AsyncFunction, got " + fns[0]->conceptType);
    auto* af = static_cast<const AsyncFunction*>(fns[0]);
    CHECK(af->name == "FetchData", "expected name 'FetchData', got '" + af->name + "'");
    PASS();
}

// 3. Parse class declaration (empty class — no inner methods to clobber name)
void test_parse_class() {
    TEST(parse_class);
    auto mod = CSharpParser::parseCSharp(R"(
class Animal {
}
)");
    CHECK(mod != nullptr, "module is null");
    auto classes = mod->getChildren("classes");
    CHECK(classes.size() == 1, "expected 1 class, got " + std::to_string(classes.size()));
    auto* cls = static_cast<const ClassDeclaration*>(classes[0]);
    CHECK(cls->name == "Animal", "expected name 'Animal', got '" + cls->name + "'");
    PASS();
}

// 4. Parse interface declaration
void test_parse_interface() {
    TEST(parse_interface);
    auto mod = CSharpParser::parseCSharp(R"(
interface IDrawable {
    void Draw();
}
)");
    CHECK(mod != nullptr, "module is null");
    auto ifaces = mod->getChildren("interfaces");
    CHECK(ifaces.size() == 1, "expected 1 interface, got " + std::to_string(ifaces.size()));
    auto* iface = static_cast<const InterfaceDeclaration*>(ifaces[0]);
    CHECK(iface->name == "IDrawable", "expected name 'IDrawable', got '" + iface->name + "'");
    PASS();
}

// 5. Using directives are skipped
void test_using_directives_skipped() {
    TEST(using_directives_skipped);
    auto mod = CSharpParser::parseCSharp(R"(
using System;
using System.Collections.Generic;

public void Main() {
    Console.WriteLine("hello");
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function (usings skipped)");
    PASS();
}

// 6. Namespace wrapper is skipped
void test_namespace_skipped() {
    TEST(namespace_skipped);
    auto mod = CSharpParser::parseCSharp(R"(
namespace MyApp {
}
)");
    CHECK(mod != nullptr, "module is null");
    // Namespace alone should not produce functions or classes
    CHECK(mod->getChildren("functions").empty(), "expected no functions");
    PASS();
}

// 7. Module target language is csharp
void test_module_target_language() {
    TEST(module_target_language);
    auto mod = CSharpParser::parseCSharp("public void Test() {\n}\n");
    CHECK(mod != nullptr, "module is null");
    CHECK(mod->targetLanguage == "csharp", "expected 'csharp', got '" + mod->targetLanguage + "'");
    CHECK(mod->name == "parsed_csharp_module", "expected module name 'parsed_csharp_module'");
    PASS();
}

// 8. Parse with diagnostics entry point
void test_parse_with_diagnostics() {
    TEST(parse_with_diagnostics);
    auto result = CSharpParser::parseCSharpWithDiagnostics(R"(
public void Test() {
    int x = 1;
}
)");
    CHECK(result.module != nullptr, "module is null from WithDiagnostics");
    auto fns = result.module->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    PASS();
}

// 9. Multiple methods
void test_parse_multiple_methods() {
    TEST(parse_multiple_methods);
    auto mod = CSharpParser::parseCSharp(R"(
public void Foo() {
    int a = 1;
}

private void Bar() {
    int b = 2;
}

static void Baz() {
    int c = 3;
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 3, "expected 3 functions, got " + std::to_string(fns.size()));
    PASS();
}

// 10. Mixed async and sync methods (top-level, no class wrapper)
void test_mixed_async_sync() {
    TEST(mixed_async_sync);
    auto mod = CSharpParser::parseCSharp(R"(
public void SyncWork() {
}

public async Task AsyncWork() {
}

public void MoreSync() {
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 3, "expected 3 functions, got " + std::to_string(fns.size()));
    CHECK(fns[0]->conceptType == "Function", "first should be Function");
    CHECK(fns[1]->conceptType == "AsyncFunction", "second should be AsyncFunction");
    CHECK(fns[2]->conceptType == "Function", "third should be Function");
    PASS();
}

// 11. Empty source produces empty module
void test_parse_empty_source() {
    TEST(parse_empty_source);
    auto mod = CSharpParser::parseCSharp("");
    CHECK(mod != nullptr, "module should not be null for empty source");
    CHECK(mod->getChildren("functions").empty(), "expected no functions");
    CHECK(mod->getChildren("classes").empty(), "expected no classes");
    PASS();
}

// 12. Comments are skipped
void test_comments_skipped() {
    TEST(comments_skipped);
    auto mod = CSharpParser::parseCSharp(R"(
// A comment
public void Execute() {
    // Inner comment
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    auto* fn = static_cast<const Function*>(fns[0]);
    CHECK(fn->name == "Execute", "expected 'Execute'");
    PASS();
}

int main() {
    std::cout << "Step 311: C# Parser Tests\n";

    test_parse_simple_method();        // 1
    test_parse_async_method();         // 2
    test_parse_class();                // 3
    test_parse_interface();            // 4
    test_using_directives_skipped();   // 5
    test_namespace_skipped();          // 6
    test_module_target_language();     // 7
    test_parse_with_diagnostics();     // 8
    test_parse_multiple_methods();     // 9
    test_mixed_async_sync();           // 10
    test_parse_empty_source();         // 11
    test_comments_skipped();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
