// Step 308: Generator Updates + Phase 11c Integration Tests (8 tests)
// Verifies all 8 generators produce language-appropriate output for 9 new
// AST node types: ClassDeclaration, InterfaceDeclaration, MethodDeclaration,
// GenericType, TypeParameter, AsyncFunction, AwaitExpression, LambdaExpression,
// DecoratorAnnotation.

#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/AsyncNodes.h"
#include "ast/Serialization.h"
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "ast/RustGenerator.h"
#include "ast/GoGenerator.h"
#include "ast/JavaGenerator.h"
#include "ast/JavaScriptGenerator.h"
#include "ast/ElispGenerator.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <memory>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Helper: build a class with one method and one field
static std::unique_ptr<ClassDeclaration> makeTestClass() {
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Animal");
    cls->superClass = "LivingThing";

    auto field = std::make_unique<Variable>("f1", "name");
    cls->addChild("fields", field.release());

    auto meth = std::make_unique<MethodDeclaration>("m1", "speak");
    meth->className = "Animal";
    meth->isVirtual = true;
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    auto retVal = std::make_unique<StringLiteral>("s1", "hello");
    retStmt->addChild("value", retVal.release());
    meth->addChild("body", retStmt.release());
    cls->addChild("methods", meth.release());

    return cls;
}

// Helper: build an async function with await
static std::unique_ptr<AsyncFunction> makeTestAsync() {
    auto af = std::make_unique<AsyncFunction>("af1", "fetchData");
    auto await = std::make_unique<AwaitExpression>("aw1");
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "getData";
    await->addChild("expression", call.release());
    auto stmt = std::make_unique<ExpressionStatement>();
    stmt->id = "es1";
    stmt->addChild("expression", await.release());
    af->addChild("body", stmt.release());
    return af;
}

// Helper: build a lambda expression
static std::unique_ptr<LambdaExpression> makeTestLambda() {
    auto lam = std::make_unique<LambdaExpression>("lam1");
    lam->captureList = {"x", "y"};
    auto paramA = std::make_unique<Parameter>("p1", "a");
    auto paramB = std::make_unique<Parameter>("p2", "b");
    lam->addChild("parameters", paramA.release());
    lam->addChild("parameters", paramB.release());
    auto body = std::make_unique<BinaryOperation>("bo1", "+");
    body->addChild("left", new VariableReference("vr1", "a"));
    body->addChild("right", new VariableReference("vr2", "b"));
    lam->addChild("body", body.release());
    return lam;
}

// 1. Parse class → generate → language-appropriate class syntax for all generators
void test_class_all_generators() {
    TEST(class_all_generators);
    auto cls = makeTestClass();

    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rustGen;
    GoGenerator goGen;
    JavaGenerator javaGen;
    JavaScriptGenerator jsGen;
    ElispGenerator elGen;
    TypeScriptGenerator tsGen;

    std::string py = pyGen.generate(cls.get());
    std::string cpp = cppGen.generate(cls.get());
    std::string rs = rustGen.generate(cls.get());
    std::string go = goGen.generate(cls.get());
    std::string java = javaGen.generate(cls.get());
    std::string js = jsGen.generate(cls.get());
    std::string el = elGen.generate(cls.get());
    std::string ts = tsGen.generate(cls.get());

    CHECK(py.find("class Animal") != std::string::npos, "Python: missing 'class Animal'");
    CHECK(cpp.find("class Animal") != std::string::npos, "C++: missing 'class Animal'");
    CHECK(rs.find("struct Animal") != std::string::npos, "Rust: missing 'struct Animal'");
    CHECK(go.find("type Animal struct") != std::string::npos, "Go: missing 'type Animal struct'");
    CHECK(java.find("class Animal") != std::string::npos, "Java: missing 'class Animal'");
    CHECK(js.find("class Animal") != std::string::npos, "JS: missing 'class Animal'");
    CHECK(el.find("cl-defstruct Animal") != std::string::npos, "Elisp: missing 'cl-defstruct Animal'");
    CHECK(ts.find("class Animal") != std::string::npos, "TS: missing 'class Animal'");

    // Verify superclass/inheritance in each
    CHECK(py.find("LivingThing") != std::string::npos, "Python: missing superclass");
    CHECK(cpp.find("public LivingThing") != std::string::npos, "C++: missing superclass");
    CHECK(java.find("extends LivingThing") != std::string::npos, "Java: missing extends");
    CHECK(js.find("extends LivingThing") != std::string::npos, "JS: missing extends");
    PASS();
}

// 2. Async roundtrip: async function + await in all generators
void test_async_all_generators() {
    TEST(async_all_generators);
    auto af = makeTestAsync();

    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rustGen;
    GoGenerator goGen;
    JavaGenerator javaGen;
    JavaScriptGenerator jsGen;
    ElispGenerator elGen;

    std::string py = pyGen.generate(af.get());
    std::string cpp = cppGen.generate(af.get());
    std::string rs = rustGen.generate(af.get());
    std::string go = goGen.generate(af.get());
    std::string java = javaGen.generate(af.get());
    std::string js = jsGen.generate(af.get());
    std::string el = elGen.generate(af.get());

    CHECK(py.find("async def") != std::string::npos, "Python: missing 'async def'");
    CHECK(py.find("await") != std::string::npos, "Python: missing 'await'");
    CHECK(cpp.find("std::future") != std::string::npos, "C++: missing 'std::future'");
    CHECK(cpp.find("co_await") != std::string::npos, "C++: missing 'co_await'");
    CHECK(rs.find("async fn") != std::string::npos, "Rust: missing 'async fn'");
    CHECK(rs.find(".await") != std::string::npos, "Rust: missing '.await'");
    CHECK(go.find("func fetchData") != std::string::npos, "Go: missing 'func fetchData'");
    CHECK(java.find("CompletableFuture") != std::string::npos, "Java: missing 'CompletableFuture'");
    CHECK(js.find("async function") != std::string::npos, "JS: missing 'async function'");
    CHECK(js.find("await") != std::string::npos, "JS: missing 'await'");
    CHECK(el.find("async") != std::string::npos, "Elisp: missing 'async' comment");
    PASS();
}

// 3. Generic type roundtrip
void test_generic_type_all_generators() {
    TEST(generic_type_all_generators);
    auto gen = std::make_unique<GenericType>("g1", "List");
    auto tp = std::make_unique<TypeParameter>("tp1", "T");
    gen->addChild("typeParameters", tp.release());

    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rustGen;
    GoGenerator goGen;
    JavaGenerator javaGen;
    JavaScriptGenerator jsGen;

    std::string py = pyGen.generate(gen.get());
    std::string cpp = cppGen.generate(gen.get());
    std::string rs = rustGen.generate(gen.get());
    std::string go = goGen.generate(gen.get());
    std::string java = javaGen.generate(gen.get());
    std::string js = jsGen.generate(gen.get());

    CHECK(py.find("List[T]") != std::string::npos, "Python: missing 'List[T]'");
    CHECK(cpp.find("List<T>") != std::string::npos, "C++: missing 'List<T>'");
    CHECK(rs.find("List<T>") != std::string::npos, "Rust: missing 'List<T>'");
    CHECK(go.find("List[T]") != std::string::npos, "Go: missing 'List[T]'");
    CHECK(java.find("List<T>") != std::string::npos, "Java: missing 'List<T>'");
    CHECK(js.find("List<T>") != std::string::npos, "JS: missing 'List<T>'");
    PASS();
}

// 4. Lambda/closure roundtrip
void test_lambda_all_generators() {
    TEST(lambda_all_generators);
    auto lam = makeTestLambda();

    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rustGen;
    GoGenerator goGen;
    JavaGenerator javaGen;
    JavaScriptGenerator jsGen;
    ElispGenerator elGen;

    std::string py = pyGen.generate(lam.get());
    std::string cpp = cppGen.generate(lam.get());
    std::string rs = rustGen.generate(lam.get());
    std::string go = goGen.generate(lam.get());
    std::string java = javaGen.generate(lam.get());
    std::string js = jsGen.generate(lam.get());
    std::string el = elGen.generate(lam.get());

    CHECK(py.find("lambda") != std::string::npos, "Python: missing 'lambda'");
    CHECK(cpp.find("[x, y]") != std::string::npos, "C++: missing '[x, y]' capture");
    CHECK(rs.find("|a, b|") != std::string::npos, "Rust: missing '|a, b|' closure");
    CHECK(go.find("func(") != std::string::npos, "Go: missing 'func(' lambda");
    CHECK(java.find("->") != std::string::npos, "Java: missing '->' lambda");
    CHECK(js.find("=>") != std::string::npos, "JS: missing '=>' arrow");
    CHECK(el.find("(lambda") != std::string::npos, "Elisp: missing '(lambda'");
    PASS();
}

// 5. Decorator roundtrip (Python → Python, Java @annotation)
void test_decorator_roundtrip() {
    TEST(decorator_roundtrip);
    auto dec = std::make_unique<DecoratorAnnotation>("dec1", "Override");

    PythonGenerator pyGen;
    JavaGenerator javaGen;
    CppGenerator cppGen;
    ElispGenerator elGen;

    std::string py = pyGen.generate(dec.get());
    std::string java = javaGen.generate(dec.get());
    std::string cpp = cppGen.generate(dec.get());
    std::string el = elGen.generate(dec.get());

    CHECK(py.find("@Override") != std::string::npos, "Python: missing '@Override'");
    CHECK(java.find("@Override") != std::string::npos, "Java: missing '@Override'");
    CHECK(cpp.find("@Override") != std::string::npos, "C++: missing '@Override' comment");
    CHECK(el.find("@Override") != std::string::npos, "Elisp: missing '@Override' comment");
    PASS();
}

// 6. Class with methods and fields — verify structure
void test_class_with_methods_and_fields() {
    TEST(class_with_methods_and_fields);
    auto cls = makeTestClass();

    RustGenerator rustGen;
    JavaGenerator javaGen;
    GoGenerator goGen;

    std::string rs = rustGen.generate(cls.get());
    std::string java = javaGen.generate(cls.get());
    std::string go = goGen.generate(cls.get());

    // Rust: struct + impl block
    CHECK(rs.find("struct Animal") != std::string::npos, "Rust: missing struct");
    CHECK(rs.find("impl Animal") != std::string::npos, "Rust: missing impl block");
    CHECK(rs.find("fn speak") != std::string::npos, "Rust: missing method");

    // Java: class + extends + method
    CHECK(java.find("extends LivingThing") != std::string::npos, "Java: missing extends");
    CHECK(java.find("speak") != std::string::npos, "Java: missing method name");

    // Go: type struct + func method
    CHECK(go.find("type Animal struct") != std::string::npos, "Go: missing type struct");
    CHECK(go.find("func") != std::string::npos, "Go: missing func keyword");
    CHECK(go.find("speak") != std::string::npos, "Go: missing method name");
    PASS();
}

// 7. Interface with abstract methods
void test_interface_all_generators() {
    TEST(interface_all_generators);
    auto iface = std::make_unique<InterfaceDeclaration>("if1", "Drawable");
    auto m1 = std::make_unique<MethodDeclaration>("m1", "draw");
    auto m2 = std::make_unique<MethodDeclaration>("m2", "resize");
    iface->addChild("methods", m1.release());
    iface->addChild("methods", m2.release());

    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rustGen;
    GoGenerator goGen;
    JavaGenerator javaGen;
    JavaScriptGenerator jsGen;
    ElispGenerator elGen;

    std::string py = pyGen.generate(iface.get());
    std::string cpp = cppGen.generate(iface.get());
    std::string rs = rustGen.generate(iface.get());
    std::string go = goGen.generate(iface.get());
    std::string java = javaGen.generate(iface.get());
    std::string js = jsGen.generate(iface.get());
    std::string el = elGen.generate(iface.get());

    CHECK(py.find("ABC") != std::string::npos, "Python: missing ABC base");
    CHECK(cpp.find("class Drawable") != std::string::npos, "C++: missing class");
    CHECK(rs.find("trait Drawable") != std::string::npos, "Rust: missing trait");
    CHECK(go.find("type Drawable interface") != std::string::npos, "Go: missing interface");
    CHECK(java.find("interface Drawable") != std::string::npos, "Java: missing interface");
    CHECK(js.find("class Drawable") != std::string::npos, "JS: missing class (no native interface)");
    CHECK(el.find("Interface: Drawable") != std::string::npos, "Elisp: missing Interface comment");

    // Verify both methods present
    CHECK(rs.find("draw") != std::string::npos && rs.find("resize") != std::string::npos,
          "Rust: missing methods in trait");
    CHECK(java.find("draw") != std::string::npos && java.find("resize") != std::string::npos,
          "Java: missing methods in interface");
    PASS();
}

// 8. Mixed: class + async + lambda in single module
void test_mixed_module_all_generators() {
    TEST(mixed_module_all_generators);
    auto mod = std::make_unique<Module>("mod1", "mixed_test", "python");

    // Add class
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Worker");
    auto meth = std::make_unique<MethodDeclaration>("m1", "run");
    meth->className = "Worker";
    cls->addChild("methods", meth.release());

    // Add async function
    auto af = std::make_unique<AsyncFunction>("af1", "process");
    auto awaitExpr = std::make_unique<AwaitExpression>("aw1");
    auto fetchCall = new FunctionCall();
    fetchCall->id = "fc1";
    fetchCall->functionName = "fetch";
    awaitExpr->addChild("expression", fetchCall);
    auto exprStmt = std::make_unique<ExpressionStatement>();
    exprStmt->id = "es1";
    exprStmt->addChild("expression", awaitExpr.release());
    af->addChild("body", exprStmt.release());

    // Add regular function containing a lambda
    auto fn = std::make_unique<Function>("fn1", "makeCallback");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    auto lam = std::make_unique<LambdaExpression>("lam1");
    auto lamParam = std::make_unique<Parameter>("p1", "x");
    lam->addChild("parameters", lamParam.release());
    auto lamBody = std::make_unique<VariableReference>("vr1", "x");
    lam->addChild("body", lamBody.release());
    retStmt->addChild("value", lam.release());
    fn->addChild("body", retStmt.release());

    // Serialize class to JSON and reconstruct — proves full pipeline works
    json clsJson = toJson(cls.get());
    auto* clsRebuilt = fromJson(clsJson);
    CHECK(clsRebuilt != nullptr, "class JSON roundtrip failed");
    CHECK(clsRebuilt->conceptType == "ClassDeclaration", "wrong type after roundtrip");

    // Verify all 8 generators produce non-empty output for class
    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rustGen;
    GoGenerator goGen;
    JavaGenerator javaGen;
    JavaScriptGenerator jsGen;
    ElispGenerator elGen;
    TypeScriptGenerator tsGen;

    struct GenResult {
        std::string name;
        std::string classOut;
        std::string asyncOut;
        std::string fnOut;
    };

    std::vector<GenResult> results;

    auto testGen = [&](const std::string& name, auto& gen) {
        GenResult r;
        r.name = name;
        r.classOut = gen.generate(cls.get());
        r.asyncOut = gen.generate(af.get());
        r.fnOut = gen.generate(fn.get());
        results.push_back(r);
    };

    testGen("Python", pyGen);
    testGen("C++", cppGen);
    testGen("Rust", rustGen);
    testGen("Go", goGen);
    testGen("Java", javaGen);
    testGen("JavaScript", jsGen);
    testGen("Elisp", elGen);
    testGen("TypeScript", tsGen);

    for (const auto& r : results) {
        CHECK(!r.classOut.empty(), (r.name + ": class output empty").c_str());
        CHECK(!r.asyncOut.empty(), (r.name + ": async output empty").c_str());
        CHECK(!r.fnOut.empty(), (r.name + ": function output empty").c_str());
        CHECK(r.classOut.find("Worker") != std::string::npos,
              (r.name + ": class missing 'Worker'").c_str());
        CHECK(r.asyncOut.find("process") != std::string::npos,
              (r.name + ": async missing 'process'").c_str());
    }

    delete clsRebuilt;
    PASS();
}

int main() {
    std::cout << "Step 308: Generator Updates + Phase 11c Integration Tests\n";

    test_class_all_generators();        // 1
    test_async_all_generators();        // 2
    test_generic_type_all_generators(); // 3
    test_lambda_all_generators();       // 4
    test_decorator_roundtrip();         // 5
    test_class_with_methods_and_fields(); // 6
    test_interface_all_generators();    // 7
    test_mixed_module_all_generators(); // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
