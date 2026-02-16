// Step 405: F# Parser Tests (12 tests)

#include "ast/FSharpParser.h"
#include "Pipeline.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/ClassDeclaration.h"
#include "ast/Statement.h"
#include "ast/AsyncNodes.h"
#include "ast/Expression.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. let name params = body -> Function
void test_let_function() {
    TEST(let_function);
    auto mod = FSharpParser::parseFSharp("let add x y = x + y\n");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    CHECK(fns[0]->conceptType == "Function", "expected Function");
    auto* fn = static_cast<Function*>(fns[0]);
    CHECK(fn->name == "add", "expected function name add");
    PASS();
}

// 2. let mutable x = 5 -> Variable with MutAnnotation
void test_let_mutable_variable() {
    TEST(let_mutable_variable);
    auto mod = FSharpParser::parseFSharp("let mutable counter = 5\n");
    CHECK(mod != nullptr, "module is null");
    auto vars = mod->getChildren("variables");
    CHECK(vars.size() == 1, "expected 1 variable");
    auto* var = static_cast<Variable*>(vars[0]);
    CHECK(var->name == "counter", "expected variable name counter");
    CHECK(!var->getChildren("annotations").empty(), "expected mutable annotation");
    PASS();
}

// 3. discriminated union -> EnumDeclaration
void test_discriminated_union() {
    TEST(discriminated_union);
    auto mod = FSharpParser::parseFSharp(
        "type Shape =\n"
        "| Circle\n"
        "| Rectangle of int\n");
    CHECK(mod != nullptr, "module is null");
    auto stmts = mod->getChildren("statements");
    EnumDeclaration* en = nullptr;
    for (auto* s : stmts) {
        if (s->conceptType == "EnumDeclaration") {
            en = static_cast<EnumDeclaration*>(s);
            break;
        }
    }
    CHECK(en != nullptr, "expected enum declaration");
    CHECK(en->name == "Shape", "expected enum name Shape");
    CHECK(en->getChildren("members").size() == 2, "expected 2 union cases");
    PASS();
}

// 4. record type -> ClassDeclaration with fields
void test_record_type() {
    TEST(record_type);
    auto mod = FSharpParser::parseFSharp("type Person = { name: string; age: int }\n");
    CHECK(mod != nullptr, "module is null");
    auto classes = mod->getChildren("classes");
    CHECK(classes.size() == 1, "expected 1 class");
    auto* cls = static_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls->name == "Person", "expected class Person");
    CHECK(cls->getChildren("fields").size() == 2, "expected 2 fields");
    PASS();
}

// 5. match x with -> IfStatement chain marker
void test_match_mapping() {
    TEST(match_mapping);
    auto mod = FSharpParser::parseFSharp(
        "let classify x =\n"
        "    match x with\n"
        "    | 0 -> \"zero\"\n"
        "    | _ -> \"other\"\n");
    CHECK(mod != nullptr, "module is null");
    auto stmts = mod->getChildren("statements");
    bool foundIf = false;
    for (auto* s : stmts) {
        if (s->conceptType == "IfStatement") foundIf = true;
    }
    CHECK(foundIf, "expected IfStatement marker for match");
    PASS();
}

// 6. async { ... } -> AsyncFunction
void test_async_computation() {
    TEST(async_computation);
    auto mod = FSharpParser::parseFSharp("let fetchData = async { return 42 }\n");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    CHECK(fns[0]->conceptType == "AsyncFunction", "expected AsyncFunction");
    auto* fn = static_cast<AsyncFunction*>(fns[0]);
    CHECK(fn->name == "fetchData", "expected async function name fetchData");
    PASS();
}

// 7. module Name -> NamespaceDeclaration
void test_module_namespace() {
    TEST(module_namespace);
    auto mod = FSharpParser::parseFSharp("module MyApp.Core\n");
    CHECK(mod != nullptr, "module is null");
    auto stmts = mod->getChildren("statements");
    NamespaceDeclaration* ns = nullptr;
    for (auto* s : stmts) {
        if (s->conceptType == "NamespaceDeclaration") {
            ns = static_cast<NamespaceDeclaration*>(s);
            break;
        }
    }
    CHECK(ns != nullptr, "expected namespace declaration");
    CHECK(ns->name == "MyApp.Core", "expected namespace MyApp.Core");
    PASS();
}

// 8. |> pipeline operators -> chained FunctionCalls
void test_pipeline_operator_calls() {
    TEST(pipeline_operator_calls);
    auto mod = FSharpParser::parseFSharp("value |> f |> g |> h\n");
    CHECK(mod != nullptr, "module is null");
    auto stmts = mod->getChildren("statements");
    int pipelineCalls = 0;
    for (auto* s : stmts) {
        if (s->conceptType == "FunctionCall") pipelineCalls++;
    }
    CHECK(pipelineCalls == 3, "expected 3 pipeline calls");
    PASS();
}

// 9. significant whitespace: indented let not top-level declaration
void test_significant_whitespace() {
    TEST(significant_whitespace);
    auto mod = FSharpParser::parseFSharp(
        "let outer x =\n"
        "    let inner = x + 1\n"
        "    inner\n");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    auto vars = mod->getChildren("variables");
    CHECK(fns.size() == 1, "expected only top-level function");
    CHECK(vars.empty(), "expected no top-level variables from indented let");
    PASS();
}

// 10. parse with diagnostics entry point
void test_parse_with_diagnostics() {
    TEST(parse_with_diagnostics);
    auto result = FSharpParser::parseFSharpWithDiagnostics("let f x = x\n");
    CHECK(result.module != nullptr, "module is null");
    CHECK(result.module->getChildren("functions").size() == 1, "expected 1 function");
    PASS();
}

// 11. mixed top-level forms
void test_mixed_toplevel_forms() {
    TEST(mixed_toplevel_forms);
    auto mod = FSharpParser::parseFSharp(
        "module Demo\n"
        "type Status = | Ok | Error of string\n"
        "type User = { id: int; name: string }\n"
        "let mutable count = 0\n"
        "let run value = value |> string\n");
    CHECK(mod != nullptr, "module is null");
    CHECK(mod->getChildren("classes").size() == 1, "expected 1 record class");
    CHECK(mod->getChildren("variables").size() == 1, "expected 1 variable");
    CHECK(mod->getChildren("functions").size() == 1, "expected 1 function");
    PASS();
}

// 12. Pipeline parse routing for fsharp/f#/fs
void test_pipeline_parse_routing() {
    TEST(pipeline_parse_routing);
    Pipeline p;
    std::vector<ParseDiagnostic> d1, d2, d3;
    auto m1 = p.parse("let inc x = x + 1\n", "fsharp", d1);
    auto m2 = p.parse("let inc x = x + 1\n", "f#", d2);
    auto m3 = p.parse("let inc x = x + 1\n", "fs", d3);
    CHECK(m1 != nullptr && m2 != nullptr && m3 != nullptr, "expected parse success for aliases");
    CHECK(m1->targetLanguage == "fsharp", "expected targetLanguage fsharp");
    CHECK(m2->targetLanguage == "fsharp", "expected alias targetLanguage fsharp");
    CHECK(m3->targetLanguage == "fsharp", "expected alias targetLanguage fsharp");
    PASS();
}

int main() {
    std::cout << "Step 405: F# Parser Tests\n";

    test_let_function();            // 1
    test_let_mutable_variable();    // 2
    test_discriminated_union();     // 3
    test_record_type();             // 4
    test_match_mapping();           // 5
    test_async_computation();       // 6
    test_module_namespace();        // 7
    test_pipeline_operator_calls(); // 8
    test_significant_whitespace();  // 9
    test_parse_with_diagnostics();  // 10
    test_mixed_toplevel_forms();    // 11
    test_pipeline_parse_routing();  // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
