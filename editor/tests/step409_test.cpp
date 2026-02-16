// Step 409: .NET Integration Tests (8 tests)

#include "Pipeline.h"
#include "ast/Annotation.h"
#include "ast/Function.h"
#include "ast/Module.h"
#include "ast/Parameter.h"

#include <iostream>
#include <string>
#include <vector>

static int passed = 0;
static int failed = 0;

#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_csharp_to_fsharp_generation() {
    TEST(csharp_to_fsharp_generation);
    Pipeline p;
    std::string src = R"(
public int Add(int a, int b) {
    return a + b;
}
)";
    auto result = p.run(src, "csharp", "fsharp");
    CHECK(result.success, "pipeline run failed");
    CHECK(!result.generatedCode.empty(), "generated code empty");
    CHECK(result.generatedCode.find("let Add") != std::string::npos, "expected F# let output");
    PASS();
}

void test_fsharp_to_csharp_generation() {
    TEST(fsharp_to_csharp_generation);
    Pipeline p;
    std::string src = "let add x y = x + y\n";
    auto result = p.run(src, "fsharp", "csharp");
    CHECK(result.success, "pipeline run failed");
    CHECK(result.generatedCode.find("public void add") != std::string::npos ||
          result.generatedCode.find("public void Add") != std::string::npos,
          "expected C# method output");
    PASS();
}

void test_csharp_to_vb_generation() {
    TEST(csharp_to_vb_generation);
    Pipeline p;
    std::string src = R"(
public void Main() {
}
)";
    auto result = p.run(src, "csharp", "vbnet");
    CHECK(result.success, "pipeline run failed");
    CHECK(result.generatedCode.find("Sub Main") != std::string::npos, "expected VB Sub");
    PASS();
}

void test_vb_to_fsharp_generation() {
    TEST(vb_to_fsharp_generation);
    Pipeline p;
    std::string src = R"(
Function Sum(a As Integer, b As Integer) As Integer
End Function
)";
    auto result = p.run(src, "vbnet", "fsharp");
    CHECK(result.success, "pipeline run failed");
    CHECK(result.generatedCode.find("let Sum") != std::string::npos, "expected F# let");
    PASS();
}

void test_python_to_fsharp_generation() {
    TEST(python_to_fsharp_generation);
    Pipeline p;
    std::string src = R"(
def greet(name):
    return name
)";
    auto result = p.run(src, "python", "fsharp");
    CHECK(result.success, "pipeline run failed");
    CHECK(result.generatedCode.find("let greet") != std::string::npos, "expected F# function");
    PASS();
}

void test_pipeline_alias_routes_for_dotnet() {
    TEST(pipeline_alias_routes_for_dotnet);
    Pipeline p;
    std::vector<ParseDiagnostic> d1, d2, d3, d4;
    auto m1 = p.parse("let add x y = x + y\n", "f#", d1);
    auto m2 = p.parse("let add x y = x + y\n", "fs", d2);
    auto m3 = p.parse("Sub Main()\nEnd Sub\n", "vb", d3);
    auto m4 = p.parse("Sub Main()\nEnd Sub\n", "vb.net", d4);
    CHECK(m1 != nullptr && m2 != nullptr && m3 != nullptr && m4 != nullptr, "alias parse route failed");
    CHECK(m1->targetLanguage == "fsharp", "f# alias target mismatch");
    CHECK(m3->targetLanguage == "vbnet", "vb alias target mismatch");
    PASS();
}

void test_shared_type_annotations_emit_for_dotnet_targets() {
    TEST(shared_type_annotations_emit_for_dotnet_targets);
    Module mod;
    mod.id = "m1";
    mod.name = "anno_mod";
    auto* fn = new Function("f1", "Convert");
    auto* t = new TypeStateAnnotation();
    t->id = "a1";
    t->state = "reified";
    fn->addChild("annotations", t);
    mod.addChild("functions", fn);

    Pipeline p;
    std::string fsharpOut = p.generate(&mod, "fsharp");
    std::string vbOut = p.generate(&mod, "vbnet");
    std::string csharpOut = p.generate(&mod, "csharp");

    CHECK(fsharpOut.find("@semanno:typestate") != std::string::npos, "fsharp semanno missing");
    CHECK(vbOut.find("@semanno:typestate") != std::string::npos, "vb semanno missing");
    CHECK(csharpOut.find("@semanno:typestate") != std::string::npos, "csharp semanno missing");
    PASS();
}

void test_pipeline_run_both_new_languages() {
    TEST(pipeline_run_both_new_languages);
    Pipeline p;

    auto fsResult = p.run("let inc x = x + 1\n", "fsharp", "fsharp");
    CHECK(fsResult.success, "fsharp run failed");
    CHECK(!fsResult.generatedCode.empty(), "fsharp generated empty");

    auto vbResult = p.run("Sub Main()\nEnd Sub\n", "vbnet", "vbnet");
    CHECK(vbResult.success, "vbnet run failed");
    CHECK(!vbResult.generatedCode.empty(), "vbnet generated empty");
    PASS();
}

int main() {
    std::cout << "Step 409: .NET Integration Tests\n";

    test_csharp_to_fsharp_generation();               // 1
    test_fsharp_to_csharp_generation();               // 2
    test_csharp_to_vb_generation();                   // 3
    test_vb_to_fsharp_generation();                   // 4
    test_python_to_fsharp_generation();               // 5
    test_pipeline_alias_routes_for_dotnet();          // 6
    test_shared_type_annotations_emit_for_dotnet_targets(); // 7
    test_pipeline_run_both_new_languages();           // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
