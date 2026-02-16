// Step 408: VB.NET Generator Tests (12 tests)

#include "ast/VBNetGenerator.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/ClassDeclaration.h"
#include "ast/Type.h"
#include "ast/Annotation.h"
#include "Pipeline.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_generate_sub_form() {
    TEST(generate_sub_form);
    Function fn("f1", "DoWork");
    fn.addChild("parameters", new Parameter("p1", "x"));
    VBNetGenerator gen;
    std::string out = gen.generate(&fn);
    CHECK(out.find("Sub DoWork(") != std::string::npos, "expected Sub form");
    CHECK(out.find("End Sub") != std::string::npos, "expected End Sub");
    PASS();
}

void test_generate_function_form() {
    TEST(generate_function_form);
    Function fn("f1", "Sum");
    fn.addChild("parameters", new Parameter("p1", "a"));
    fn.addChild("parameters", new Parameter("p2", "b"));
    fn.setChild("returnType", new PrimitiveType("t1", "int"));
    VBNetGenerator gen;
    std::string out = gen.generate(&fn);
    CHECK(out.find("Function Sum(") != std::string::npos, "expected Function form");
    CHECK(out.find("As Integer") != std::string::npos, "expected return type");
    CHECK(out.find("End Function") != std::string::npos, "expected End Function");
    PASS();
}

void test_generate_class() {
    TEST(generate_class);
    ClassDeclaration cls("c1", "Person");
    cls.addChild("fields", new Variable("v1", "Name"));
    VBNetGenerator gen;
    std::string out = gen.generate(&cls);
    CHECK(out.find("Class Person") != std::string::npos, "expected class declaration");
    CHECK(out.find("End Class") != std::string::npos, "expected End Class");
    PASS();
}

void test_generate_interface() {
    TEST(generate_interface);
    InterfaceDeclaration iface("i1", "IWorker");
    VBNetGenerator gen;
    std::string out = gen.generate(&iface);
    CHECK(out.find("Interface IWorker") != std::string::npos, "expected interface declaration");
    CHECK(out.find("End Interface") != std::string::npos, "expected End Interface");
    PASS();
}

void test_generate_module_namespace() {
    TEST(generate_module_namespace);
    NamespaceDeclaration ns("n1", "Utils");
    VBNetGenerator gen;
    std::string out = gen.generate(&ns);
    CHECK(out.find("Module Utils") != std::string::npos, "expected module declaration");
    CHECK(out.find("End Module") != std::string::npos, "expected End Module");
    PASS();
}

void test_generate_dim_variable() {
    TEST(generate_dim_variable);
    Variable v("v1", "count");
    v.setChild("type", new PrimitiveType("t1", "int"));
    v.setChild("initializer", new IntegerLiteral("i1", 5));
    VBNetGenerator gen;
    std::string out = gen.generate(&v);
    CHECK(out.find("Dim count As Integer = 5") != std::string::npos, "expected Dim As Integer");
    PASS();
}

void test_generate_if_statement() {
    TEST(generate_if_statement);
    IfStatement ifs;
    ifs.id = "if1";
    ifs.setChild("condition", new BooleanLiteral("b1", true));
    ifs.addChild("thenBranch", new Return());
    VBNetGenerator gen;
    std::string out = gen.generate(&ifs);
    CHECK(out.find("If True Then") != std::string::npos, "expected If Then");
    CHECK(out.find("End If") != std::string::npos, "expected End If");
    PASS();
}

void test_generate_for_each() {
    TEST(generate_for_each);
    ForLoop loop;
    loop.id = "for1";
    loop.iteratorName = "item";
    loop.setChild("iterable", new VariableReference("vr1", "items"));
    VBNetGenerator gen;
    std::string out = gen.generate(&loop);
    CHECK(out.find("For Each item In items") != std::string::npos, "expected For Each");
    CHECK(out.find("Next") != std::string::npos, "expected Next");
    PASS();
}

void test_type_mapping() {
    TEST(type_mapping);
    VBNetGenerator gen;
    PrimitiveType tInt("t1", "int");
    PrimitiveType tDouble("t2", "double");
    PrimitiveType tString("t3", "string");
    PrimitiveType tBool("t4", "bool");
    CHECK(gen.generate(&tInt) == "Integer", "int->Integer");
    CHECK(gen.generate(&tDouble) == "Double", "double->Double");
    CHECK(gen.generate(&tString) == "String", "string->String");
    CHECK(gen.generate(&tBool) == "Boolean", "bool->Boolean");
    PASS();
}

void test_comment_prefix() {
    TEST(comment_prefix);
    VBNetGenerator gen;
    CHECK(gen.commentPrefix() == "' ", "expected apostrophe comment prefix");
    PASS();
}

void test_semanno_output() {
    TEST(semanno_output);
    Function fn("f1", "Compute");
    auto* risk = new RiskAnnotation();
    risk->id = "r1";
    risk->level = "high";
    fn.addChild("annotations", risk);
    VBNetGenerator gen;
    std::string out = gen.generate(&fn);
    CHECK(out.find("@semanno:risk") != std::string::npos, "expected semanno risk output");
    PASS();
}

void test_pipeline_generate_routing() {
    TEST(pipeline_generate_routing);
    Module mod;
    mod.id = "m1";
    mod.name = "demo";
    mod.addChild("functions", new Function("f1", "Main"));
    Pipeline p;
    std::string o1 = p.generate(&mod, "vbnet");
    std::string o2 = p.generate(&mod, "vb");
    std::string o3 = p.generate(&mod, "vb.net");
    CHECK(o1.find("Sub Main") != std::string::npos, "vbnet route");
    CHECK(o2.find("Sub Main") != std::string::npos, "vb route");
    CHECK(o3.find("Sub Main") != std::string::npos, "vb.net route");
    PASS();
}

int main() {
    std::cout << "Step 408: VB.NET Generator Tests\n";

    test_generate_sub_form();         // 1
    test_generate_function_form();    // 2
    test_generate_class();            // 3
    test_generate_interface();        // 4
    test_generate_module_namespace(); // 5
    test_generate_dim_variable();     // 6
    test_generate_if_statement();     // 7
    test_generate_for_each();         // 8
    test_type_mapping();              // 9
    test_comment_prefix();            // 10
    test_semanno_output();            // 11
    test_pipeline_generate_routing(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
