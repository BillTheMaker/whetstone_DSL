// Step 309: Kotlin Parser Tests (12 tests)
// Verifies KotlinParser parses Kotlin source into Whetstone AST:
// fun, suspend fun → AsyncFunction, class, data class → ClassDeclaration,
// val/var → Variable, interface detection.

#include "ast/KotlinParser.h"
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

// 1. Parse a simple Kotlin function
void test_parse_simple_function() {
    TEST(parse_simple_function);
    auto mod = KotlinParser::parseKotlin(R"(
fun greet() {
    println("Hello")
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function, got " + std::to_string(fns.size()));
    CHECK(fns[0]->conceptType == "Function", "expected Function, got " + fns[0]->conceptType);
    auto* fn = static_cast<const Function*>(fns[0]);
    CHECK(fn->name == "greet", "expected name 'greet', got '" + fn->name + "'");
    PASS();
}

// 2. Parse multiple functions
void test_parse_multiple_functions() {
    TEST(parse_multiple_functions);
    auto mod = KotlinParser::parseKotlin(R"(
fun foo() {
    val x = 1
}

fun bar() {
    val y = 2
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 2, "expected 2 functions, got " + std::to_string(fns.size()));
    PASS();
}

// 3. Parse suspend fun → AsyncFunction
void test_parse_suspend_function() {
    TEST(parse_suspend_function);
    auto mod = KotlinParser::parseKotlin(R"(
suspend fun fetchData() {
    val result = api.call()
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    CHECK(fns[0]->conceptType == "AsyncFunction", "expected AsyncFunction, got " + fns[0]->conceptType);
    auto* af = static_cast<const AsyncFunction*>(fns[0]);
    CHECK(af->name == "fetchData", "expected name 'fetchData', got '" + af->name + "'");
    PASS();
}

// 4. Parse class declaration
void test_parse_class() {
    TEST(parse_class);
    auto mod = KotlinParser::parseKotlin(R"(
class Animal {
    fun speak() {
        println("...")
    }
}
)");
    CHECK(mod != nullptr, "module is null");
    auto classes = mod->getChildren("classes");
    CHECK(classes.size() == 1, "expected 1 class, got " + std::to_string(classes.size()));
    CHECK(classes[0]->conceptType == "ClassDeclaration", "expected ClassDeclaration");
    auto* cls = static_cast<const ClassDeclaration*>(classes[0]);
    CHECK(cls->name == "Animal", "expected name 'Animal', got '" + cls->name + "'");
    PASS();
}

// 5. Parse data class
void test_parse_data_class() {
    TEST(parse_data_class);
    auto mod = KotlinParser::parseKotlin(R"(
data class Point(val x: Int, val y: Int) {
}
)");
    CHECK(mod != nullptr, "module is null");
    auto classes = mod->getChildren("classes");
    CHECK(classes.size() == 1, "expected 1 class");
    auto* cls = static_cast<const ClassDeclaration*>(classes[0]);
    CHECK(cls->name == "Point", "expected name 'Point', got '" + cls->name + "'");
    PASS();
}

// 6. Parse val/var → Variable
void test_parse_variables() {
    TEST(parse_variables);
    auto mod = KotlinParser::parseKotlin(R"(
val greeting = "hello"
var counter = 0
)");
    CHECK(mod != nullptr, "module is null");
    auto vars = mod->getChildren("variables");
    CHECK(vars.size() == 2, "expected 2 variables, got " + std::to_string(vars.size()));
    auto* v1 = static_cast<const Variable*>(vars[0]);
    auto* v2 = static_cast<const Variable*>(vars[1]);
    CHECK(v1->name == "greeting", "expected 'greeting', got '" + v1->name + "'");
    CHECK(v2->name == "counter", "expected 'counter', got '" + v2->name + "'");
    PASS();
}

// 7. Module has correct target language
void test_module_target_language() {
    TEST(module_target_language);
    auto mod = KotlinParser::parseKotlin("fun main() {\n}\n");
    CHECK(mod != nullptr, "module is null");
    CHECK(mod->targetLanguage == "kotlin", "expected 'kotlin', got '" + mod->targetLanguage + "'");
    CHECK(mod->name == "parsed_kotlin_module", "expected module name 'parsed_kotlin_module'");
    PASS();
}

// 8. Parse with diagnostics entry point
void test_parse_with_diagnostics() {
    TEST(parse_with_diagnostics);
    auto result = KotlinParser::parseKotlinWithDiagnostics(R"(
fun test() {
    val x = 1
}
)");
    CHECK(result.module != nullptr, "module is null from WithDiagnostics");
    auto fns = result.module->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function from diagnostics parse");
    PASS();
}

// 9. Mixed: class + function + variable
void test_parse_mixed_toplevel() {
    TEST(parse_mixed_toplevel);
    auto mod = KotlinParser::parseKotlin(R"(
val VERSION = "1.0"

class Config {
    fun load() {}
}

fun main() {
    println("start")
}
)");
    CHECK(mod != nullptr, "module is null");
    auto vars = mod->getChildren("variables");
    auto classes = mod->getChildren("classes");
    auto fns = mod->getChildren("functions");
    CHECK(vars.size() == 1, "expected 1 variable, got " + std::to_string(vars.size()));
    CHECK(classes.size() == 1, "expected 1 class, got " + std::to_string(classes.size()));
    CHECK(fns.size() == 1, "expected 1 function, got " + std::to_string(fns.size()));
    PASS();
}

// 10. Empty source produces empty module
void test_parse_empty_source() {
    TEST(parse_empty_source);
    auto mod = KotlinParser::parseKotlin("");
    CHECK(mod != nullptr, "module should not be null for empty source");
    CHECK(mod->getChildren("functions").empty(), "expected no functions");
    CHECK(mod->getChildren("classes").empty(), "expected no classes");
    CHECK(mod->getChildren("variables").empty(), "expected no variables");
    PASS();
}

// 11. Comments are skipped
void test_comments_skipped() {
    TEST(comments_skipped);
    auto mod = KotlinParser::parseKotlin(R"(
// This is a comment
fun doWork() {
    // Another comment
}
)");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    auto* fn = static_cast<const Function*>(fns[0]);
    CHECK(fn->name == "doWork", "expected 'doWork'");
    PASS();
}

// 12. Suspend + regular functions combined
void test_mixed_suspend_and_regular() {
    TEST(mixed_suspend_and_regular);
    auto mod = KotlinParser::parseKotlin(R"(
fun syncWork() {
    val x = 1
}

suspend fun asyncWork() {
    delay(1000)
}

fun moreSync() {
    println("done")
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

int main() {
    std::cout << "Step 309: Kotlin Parser Tests\n";

    test_parse_simple_function();       // 1
    test_parse_multiple_functions();    // 2
    test_parse_suspend_function();      // 3
    test_parse_class();                 // 4
    test_parse_data_class();            // 5
    test_parse_variables();             // 6
    test_module_target_language();      // 7
    test_parse_with_diagnostics();      // 8
    test_parse_mixed_toplevel();        // 9
    test_parse_empty_source();          // 10
    test_comments_skipped();            // 11
    test_mixed_suspend_and_regular();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
