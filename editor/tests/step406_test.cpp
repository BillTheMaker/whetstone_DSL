// Step 406: F# Generator Tests (12 tests)

#include "ast/FSharpGenerator.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/ClassDeclaration.h"
#include "ast/Type.h"
#include "ast/Annotation.h"
#include "ast/AsyncNodes.h"
#include "Pipeline.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_generate_let_function() {
    TEST(generate_let_function);
    Function fn("fn1", "add");
    fn.addChild("parameters", new Parameter("p1", "x"));
    fn.addChild("parameters", new Parameter("p2", "y"));
    auto* ret = new Return();
    ret->id = "ret1";
    ret->setChild("value", new BinaryOperation("b1", "+"));
    fn.addChild("body", ret);

    FSharpGenerator gen;
    std::string out = gen.generate(&fn);
    CHECK(out.find("let add x y =") != std::string::npos, "expected let function signature");
    PASS();
}

void test_generate_mutable_variable() {
    TEST(generate_mutable_variable);
    Variable v("v1", "counter");
    auto* mut = new MutAnnotation();
    mut->id = "m1";
    v.addChild("annotations", mut);

    FSharpGenerator gen;
    std::string out = gen.generate(&v);
    CHECK(out.find("let mutable counter") != std::string::npos, "expected mutable variable");
    PASS();
}

void test_generate_discriminated_union() {
    TEST(generate_discriminated_union);
    EnumDeclaration en("e1", "Shape");
    en.addChild("members", new EnumMember("c1", "Circle"));
    en.addChild("members", new EnumMember("c2", "Rectangle"));

    FSharpGenerator gen;
    std::string out = gen.generate(&en);
    CHECK(out.find("type Shape =") != std::string::npos, "expected type declaration");
    CHECK(out.find("| Circle") != std::string::npos, "expected Circle case");
    CHECK(out.find("| Rectangle") != std::string::npos, "expected Rectangle case");
    PASS();
}

void test_generate_record_class() {
    TEST(generate_record_class);
    ClassDeclaration cls("c1", "Person");
    cls.addChild("fields", new Variable("f1", "name"));
    cls.addChild("fields", new Variable("f2", "age"));

    FSharpGenerator gen;
    std::string out = gen.generate(&cls);
    CHECK(out.find("type Person = {") != std::string::npos, "expected record type");
    CHECK(out.find("name: obj") != std::string::npos, "expected name field");
    CHECK(out.find("age: obj") != std::string::npos, "expected age field");
    PASS();
}

void test_generate_async_function() {
    TEST(generate_async_function);
    AsyncFunction fn("a1", "fetchData");
    fn.addChild("body", new Return());

    FSharpGenerator gen;
    std::string out = gen.generate(&fn);
    CHECK(out.find("let fetchData = async {") != std::string::npos, "expected async function form");
    PASS();
}

void test_generate_module_namespace() {
    TEST(generate_module_namespace);
    NamespaceDeclaration ns("n1", "MyApp.Core");
    FSharpGenerator gen;
    std::string out = gen.generate(&ns);
    CHECK(out == "module MyApp.Core", "expected module declaration");
    PASS();
}

void test_generate_if_statement() {
    TEST(generate_if_statement);
    IfStatement ifs;
    ifs.id = "if1";
    ifs.setChild("condition", new BooleanLiteral("b1", true));
    auto* thenRet = new Return();
    thenRet->id = "ret1";
    thenRet->setChild("value", new StringLiteral("s1", "yes"));
    ifs.addChild("thenBranch", thenRet);

    FSharpGenerator gen;
    std::string out = gen.generate(&ifs);
    CHECK(out.find("if true then") != std::string::npos, "expected if then");
    PASS();
}

void test_generate_pipeline_call_marker() {
    TEST(generate_pipeline_call_marker);
    FunctionCall call;
    call.id = "call1";
    call.functionName = "pipeline";
    FSharpGenerator gen;
    std::string out = gen.generate(&call);
    CHECK(out.find("|>") != std::string::npos, "expected pipeline operator");
    PASS();
}

void test_type_mapping() {
    TEST(type_mapping);
    PrimitiveType i("t1", "int");
    PrimitiveType s("t2", "string");
    PrimitiveType v("t3", "void");
    OptionalType opt;
    opt.id = "opt1";
    opt.setChild("innerType", new CustomType("ct1", "result"));

    FSharpGenerator gen;
    CHECK(gen.generate(&i) == "int", "int mapping");
    CHECK(gen.generate(&s) == "string", "string mapping");
    CHECK(gen.generate(&v) == "unit", "void->unit mapping");
    CHECK(gen.generate(&opt) == "result option", "optional mapping");
    PASS();
}

void test_comment_prefix() {
    TEST(comment_prefix);
    FSharpGenerator gen;
    CHECK(gen.commentPrefix() == "// ", "expected // comment prefix");
    PASS();
}

void test_semanno_annotation_output() {
    TEST(semanno_annotation_output);
    Function fn("f1", "compute");
    auto* c = new ComplexityAnnotation();
    c->id = "a1";
    c->timeComplexity = "O(n)";
    fn.addChild("annotations", c);

    FSharpGenerator gen;
    std::string out = gen.generate(&fn);
    CHECK(out.find("@semanno:complexity") != std::string::npos, "expected semanno complexity output");
    PASS();
}

void test_pipeline_generate_routing() {
    TEST(pipeline_generate_routing);
    Module mod;
    mod.id = "m1";
    mod.name = "demo";
    auto* fn = new Function("f1", "run");
    mod.addChild("functions", fn);

    Pipeline p;
    std::string o1 = p.generate(&mod, "fsharp");
    std::string o2 = p.generate(&mod, "f#");
    std::string o3 = p.generate(&mod, "fs");
    CHECK(o1.find("let run") != std::string::npos, "fsharp route");
    CHECK(o2.find("let run") != std::string::npos, "f# route");
    CHECK(o3.find("let run") != std::string::npos, "fs route");
    PASS();
}

int main() {
    std::cout << "Step 406: F# Generator Tests\n";

    test_generate_let_function();      // 1
    test_generate_mutable_variable();  // 2
    test_generate_discriminated_union();// 3
    test_generate_record_class();      // 4
    test_generate_async_function();    // 5
    test_generate_module_namespace();  // 6
    test_generate_if_statement();      // 7
    test_generate_pipeline_call_marker();// 8
    test_type_mapping();               // 9
    test_comment_prefix();             // 10
    test_semanno_annotation_output();  // 11
    test_pipeline_generate_routing();  // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
