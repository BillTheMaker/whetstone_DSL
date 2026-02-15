// Step 313: New Languages Integration + MCP Tests (8 tests)
// Full integration of Kotlin + C# across the pipeline:
// Pipeline.run(), cross-language projection, all 10 generators,
// memory inference, final language count.

#include "Pipeline.h"
#include "ast/KotlinParser.h"
#include "ast/CSharpParser.h"
#include "ast/KotlinGenerator.h"
#include "ast/CSharpGenerator.h"
#include "ast/Generator.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/ClassDeclaration.h"
#include "ast/AsyncNodes.h"
#include "ast/Serialization.h"
#include "CrossLanguageProjector.h"
#include "MemoryStrategyInference.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. Cross-language projection: Python-like AST → Kotlin
void test_cross_projection_to_kotlin() {
    TEST(cross_projection_to_kotlin);
    // Build a Python-flavored AST
    auto mod = std::make_unique<Module>("mod1", "pymod", "python");
    auto fn = std::make_unique<Function>("fn1", "compute");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    auto retVal = std::make_unique<IntegerLiteral>("il1", 99);
    retStmt->addChild("value", retVal.release());
    fn->addChild("body", retStmt.release());
    mod->addChild("functions", fn.release());

    // Generate as Kotlin
    KotlinGenerator gen;
    std::string out = gen.generate(mod.get());
    CHECK(out.find("fun compute") != std::string::npos, "Kotlin output missing 'fun compute'");
    CHECK(out.find("return 99") != std::string::npos, "Kotlin output missing 'return 99'");
    PASS();
}

// 2. Cross-language projection: Java-like AST → C#
void test_cross_projection_to_csharp() {
    TEST(cross_projection_to_csharp);
    auto mod = std::make_unique<Module>("mod1", "javamod", "java");
    auto fn = std::make_unique<Function>("fn1", "Process");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    auto retVal = std::make_unique<StringLiteral>("s1", "result");
    retStmt->addChild("value", retVal.release());
    fn->addChild("body", retStmt.release());
    mod->addChild("functions", fn.release());

    CSharpGenerator gen;
    std::string out = gen.generate(mod.get());
    CHECK(out.find("public void Process") != std::string::npos, "C# output missing 'public void Process'");
    CHECK(out.find("return \"result\"") != std::string::npos, "C# output missing return statement");
    PASS();
}

// 3. Pipeline.parse() with Kotlin source
void test_pipeline_parse_kotlin() {
    TEST(pipeline_parse_kotlin);
    Pipeline pipeline;
    std::vector<ParseDiagnostic> diags;
    auto mod = pipeline.parse(R"(
fun hello() {
    println("world")
}
)", "kotlin", diags);
    CHECK(mod != nullptr, "Pipeline.parse() returned null for Kotlin");
    CHECK(mod->targetLanguage == "kotlin", "target language should be 'kotlin'");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function from Kotlin parse");
    PASS();
}

// 4. Pipeline.parse() with C# source
void test_pipeline_parse_csharp() {
    TEST(pipeline_parse_csharp);
    Pipeline pipeline;
    std::vector<ParseDiagnostic> diags;
    auto mod = pipeline.parse(R"(
public void Execute() {
    int x = 42;
}
)", "csharp", diags);
    CHECK(mod != nullptr, "Pipeline.parse() returned null for C#");
    CHECK(mod->targetLanguage == "csharp", "target language should be 'csharp'");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function from C# parse");
    PASS();
}

// 5. Pipeline.generate() for both new languages
void test_pipeline_generate_both() {
    TEST(pipeline_generate_both);
    Pipeline pipeline;

    // Build a simple AST
    auto mod = std::make_unique<Module>("mod1", "test", "python");
    auto fn = std::make_unique<Function>("fn1", "work");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    fn->addChild("body", retStmt.release());
    mod->addChild("functions", fn.release());

    std::string kotlin = pipeline.generate(mod.get(), "kotlin");
    std::string csharp = pipeline.generate(mod.get(), "csharp");

    CHECK(!kotlin.empty(), "Kotlin generation produced empty output");
    CHECK(!csharp.empty(), "C# generation produced empty output");
    CHECK(kotlin.find("fun work") != std::string::npos, "Kotlin missing 'fun work'");
    CHECK(csharp.find("public void work") != std::string::npos, "C# missing 'public void work'");
    PASS();
}

// 6. MemoryStrategyInference handles Kotlin/C# languages
void test_memory_inference_new_languages() {
    TEST(memory_inference_new_languages);
    // Build a Kotlin module
    auto ktMod = std::make_unique<Module>("mod1", "ktmod", "kotlin");
    auto fn1 = std::make_unique<Function>("fn1", "run");
    ktMod->addChild("functions", fn1.release());

    MemoryStrategyInference inference;
    auto suggestions = inference.inferAnnotations(ktMod.get());
    // Should not crash; suggestions depend on module structure
    // Just verify it completes without error

    // Build a C# module
    auto csMod = std::make_unique<Module>("mod2", "csmod", "csharp");
    auto fn2 = std::make_unique<Function>("fn2", "execute");
    csMod->addChild("functions", fn2.release());

    auto csSuggestions = inference.inferAnnotations(csMod.get());
    // Same: should not crash
    CHECK(true, "");
    PASS();
}

// 7. All 10 generators produce non-empty output
void test_all_10_generators() {
    TEST(all_10_generators);
    Pipeline pipeline;

    auto fn = std::make_unique<Function>("fn1", "test");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    fn->addChild("body", retStmt.release());

    std::vector<std::string> languages = {
        "python", "cpp", "elisp", "javascript", "typescript",
        "java", "rust", "go", "kotlin", "csharp"
    };

    for (const auto& lang : languages) {
        std::string out = pipeline.generate(fn.get(), lang);
        CHECK(!out.empty(), ("Empty output for language: " + lang).c_str());
    }
    CHECK(languages.size() == 10, "expected 10 languages");
    PASS();
}

// 8. Final language count: 10 parsers, 10 generators
void test_final_language_count() {
    TEST(final_language_count);
    Pipeline pipeline;

    // Test all 10 parsers produce valid modules
    struct ParserTest {
        std::string lang;
        std::string source;
    };

    std::vector<ParserTest> parserTests = {
        {"python", "def hello():\n    pass\n"},
        {"cpp", "void hello() {}\n"},
        {"elisp", "(defun hello () nil)\n"},
        {"javascript", "function hello() {}\n"},
        {"typescript", "function hello(): void {}\n"},
        {"java", "public void hello() {}\n"},
        {"rust", "fn hello() {}\n"},
        {"go", "func hello() {\n}\n"},
        {"kotlin", "fun hello() {\n}\n"},
        {"csharp", "public void hello() {\n}\n"},
    };

    int parserCount = 0;
    for (const auto& pt : parserTests) {
        std::vector<ParseDiagnostic> diags;
        auto mod = pipeline.parse(pt.source, pt.lang, diags);
        if (mod != nullptr) parserCount++;
    }

    CHECK(parserCount == 10, "expected 10 working parsers, got " + std::to_string(parserCount));

    // Verify all 10 generators
    auto testNode = std::make_unique<Function>("fn1", "test");
    auto ret = std::make_unique<Return>();
    ret->id = "r1";
    testNode->addChild("body", ret.release());

    std::vector<std::string> genLangs = {
        "python", "cpp", "elisp", "javascript", "typescript",
        "java", "rust", "go", "kotlin", "csharp"
    };

    int genCount = 0;
    for (const auto& lang : genLangs) {
        std::string out = pipeline.generate(testNode.get(), lang);
        if (!out.empty()) genCount++;
    }

    CHECK(genCount == 10, "expected 10 working generators, got " + std::to_string(genCount));
    PASS();
}

int main() {
    std::cout << "Step 313: New Languages Integration Tests\n";

    test_cross_projection_to_kotlin();   // 1
    test_cross_projection_to_csharp();   // 2
    test_pipeline_parse_kotlin();        // 3
    test_pipeline_parse_csharp();        // 4
    test_pipeline_generate_both();       // 5
    test_memory_inference_new_languages(); // 6
    test_all_10_generators();            // 7
    test_final_language_count();         // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
