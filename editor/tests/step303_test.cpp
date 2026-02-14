// Step 303: New AST Nodes — Async/Lambda/Decorator (12 tests)
// Tests AsyncFunction, AwaitExpression, LambdaExpression, DecoratorAnnotation:
// construction, properties, children, serialization roundtrip, compact AST names.

#include "ast/AsyncNodes.h"
#include "ast/Serialization.h"
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

// 1. AsyncFunction construction and isAsync flag
void test_async_function_construction() {
    TEST(async_function_construction);
    AsyncFunction af("af1", "fetchData");

    CHECK(af.conceptType == "AsyncFunction", "wrong conceptType");
    CHECK(af.id == "af1", "wrong id");
    CHECK(af.name == "fetchData", "wrong name");
    CHECK(af.isAsync == true, "isAsync should default to true");
    PASS();
}

// 2. AsyncFunction inherits Function children (parameters, body)
void test_async_function_children() {
    TEST(async_function_children);
    auto af = std::make_unique<AsyncFunction>("af1", "loadItems");

    auto param = new Parameter();
    param->id = "p1";
    param->name = "url";
    af->addChild("parameters", param);

    auto ret = new Return();
    ret->id = "r1";
    af->addChild("body", ret);

    CHECK(af->getChildren("parameters").size() == 1, "expected 1 parameter");
    CHECK(af->getChildren("body").size() == 1, "expected 1 body stmt");
    PASS();
}

// 3. AwaitExpression construction and child expression
void test_await_expression() {
    TEST(await_expression);
    AwaitExpression aw("aw1");

    CHECK(aw.conceptType == "AwaitExpression", "wrong conceptType");
    CHECK(aw.id == "aw1", "wrong id");

    auto call = []{ auto* c = new FunctionCall(); c->id = "fc1"; c->functionName = "fetch"; return c; }();
    aw.setChild("expression", call);
    auto* expr = aw.getChild("expression");
    CHECK(expr != nullptr, "expression child should exist");
    CHECK(expr->conceptType == "FunctionCall", "wrong expression type");
    PASS();
}

// 4. LambdaExpression construction with capture list
void test_lambda_construction() {
    TEST(lambda_construction);
    LambdaExpression lam("lam1");
    lam.captureList = {"x", "y", "z"};

    CHECK(lam.conceptType == "LambdaExpression", "wrong conceptType");
    CHECK(lam.captureList.size() == 3, "wrong capture count");
    CHECK(lam.captureList[0] == "x", "capture[0] wrong");
    CHECK(lam.captureList[2] == "z", "capture[2] wrong");
    PASS();
}

// 5. LambdaExpression with parameters and body children
void test_lambda_children() {
    TEST(lambda_children);
    auto lam = std::make_unique<LambdaExpression>("lam1");

    auto param = new Parameter();
    param->id = "p1";
    param->name = "item";
    lam->addChild("parameters", param);

    auto ret = new Return();
    ret->id = "r1";
    lam->addChild("body", ret);

    CHECK(lam->getChildren("parameters").size() == 1, "expected 1 param");
    CHECK(lam->getChildren("body").size() == 1, "expected 1 body stmt");
    PASS();
}

// 6. DecoratorAnnotation construction and name
void test_decorator_construction() {
    TEST(decorator_construction);
    DecoratorAnnotation dec("dec1", "property");

    CHECK(dec.conceptType == "DecoratorAnnotation", "wrong conceptType");
    CHECK(dec.name == "property", "wrong name");

    auto arg = new StringLiteral("s1", "cached");
    dec.addChild("arguments", arg);
    CHECK(dec.getChildren("arguments").size() == 1, "expected 1 argument");
    PASS();
}

// 7. AsyncFunction JSON roundtrip
void test_async_json_roundtrip() {
    TEST(async_json_roundtrip);
    auto af = std::make_unique<AsyncFunction>("af1", "download");
    af->isAsync = true;

    auto param = new Parameter();
    param->id = "p1";
    param->name = "url";
    af->addChild("parameters", param);

    json j = toJson(af.get());
    CHECK(j["concept"] == "AsyncFunction", "concept wrong");
    CHECK(j["properties"]["name"] == "download", "name wrong");
    CHECK(j["properties"]["isAsync"] == true, "isAsync wrong");

    ASTNode* restored = fromJson(j);
    CHECK(restored != nullptr, "fromJson null");
    CHECK(restored->conceptType == "AsyncFunction", "restored type");
    auto* raf = static_cast<AsyncFunction*>(restored);
    CHECK(raf->name == "download", "restored name");
    CHECK(raf->isAsync == true, "restored isAsync");
    CHECK(restored->getChildren("parameters").size() == 1, "restored params");
    delete restored;
    PASS();
}

// 8. AwaitExpression JSON roundtrip
void test_await_json_roundtrip() {
    TEST(await_json_roundtrip);
    auto aw = std::make_unique<AwaitExpression>("aw1");
    auto call = []{ auto* c = new FunctionCall(); c->id = "fc1"; c->functionName = "getData"; return c; }();
    aw->setChild("expression", call);

    json j = toJson(aw.get());
    CHECK(j["concept"] == "AwaitExpression", "concept wrong");

    ASTNode* restored = fromJson(j);
    CHECK(restored != nullptr, "fromJson null");
    CHECK(restored->conceptType == "AwaitExpression", "restored type");
    auto* expr = restored->getChild("expression");
    CHECK(expr != nullptr, "restored expression child");
    CHECK(expr->conceptType == "FunctionCall", "restored expression type");
    delete restored;
    PASS();
}

// 9. LambdaExpression JSON roundtrip preserves captureList
void test_lambda_json_roundtrip() {
    TEST(lambda_json_roundtrip);
    auto lam = std::make_unique<LambdaExpression>("lam1");
    lam->captureList = {"a", "b"};

    auto param = new Parameter();
    param->id = "p1";
    param->name = "x";
    lam->addChild("parameters", param);

    json j = toJson(lam.get());
    CHECK(j["properties"]["captureList"].is_array(), "captureList should be array");
    CHECK(j["properties"]["captureList"].size() == 2, "captureList size");

    ASTNode* restored = fromJson(j);
    CHECK(restored->conceptType == "LambdaExpression", "restored type");
    auto* rl = static_cast<LambdaExpression*>(restored);
    CHECK(rl->captureList.size() == 2, "restored captureList size");
    CHECK(rl->captureList[0] == "a", "restored capture[0]");
    CHECK(rl->captureList[1] == "b", "restored capture[1]");
    CHECK(restored->getChildren("parameters").size() == 1, "restored params");
    delete restored;
    PASS();
}

// 10. DecoratorAnnotation JSON roundtrip
void test_decorator_json_roundtrip() {
    TEST(decorator_json_roundtrip);
    auto dec = std::make_unique<DecoratorAnnotation>("dec1", "cache");
    auto arg = new StringLiteral("s1", "60s");
    dec->addChild("arguments", arg);

    json j = toJson(dec.get());
    CHECK(j["properties"]["name"] == "cache", "name wrong");

    ASTNode* restored = fromJson(j);
    CHECK(restored->conceptType == "DecoratorAnnotation", "restored type");
    auto* rd = static_cast<DecoratorAnnotation*>(restored);
    CHECK(rd->name == "cache", "restored name");
    CHECK(restored->getChildren("arguments").size() == 1, "restored args");
    delete restored;
    PASS();
}

// 11. AsyncFunction in module with AwaitExpression in body
void test_async_with_await_in_module() {
    TEST(async_with_await_in_module);
    auto mod = std::make_unique<Module>();
    mod->name = "api";

    auto af = new AsyncFunction("af1", "fetchUser");
    auto aw = new AwaitExpression("aw1");
    aw->setChild("expression", []{ auto* c = new FunctionCall(); c->id = "fc1"; c->functionName = "httpGet"; return c; }());
    auto exprStmt = new ExpressionStatement();
    exprStmt->id = "es1";
    exprStmt->setChild("expression", aw);
    af->addChild("body", exprStmt);
    mod->addChild("functions", af);

    // Roundtrip the whole module
    json j = toJson(mod.get());
    ASTNode* restored = fromJson(j);
    CHECK(restored->conceptType == "Module", "module type");
    auto funcs = restored->getChildren("functions");
    CHECK(funcs.size() == 1, "1 function");
    CHECK(funcs[0]->conceptType == "AsyncFunction", "function is AsyncFunction");
    auto* resAf = static_cast<AsyncFunction*>(funcs[0]);
    CHECK(resAf->name == "fetchUser", "function name");
    auto body = resAf->getChildren("body");
    CHECK(!body.empty(), "body not empty");
    delete restored;
    PASS();
}

// 12. Lambda with captures inside a function body
void test_lambda_in_function() {
    TEST(lambda_in_function);
    auto mod = std::make_unique<Module>();
    mod->name = "utils";

    auto fn = new Function("fn1", "process");
    auto lam = new LambdaExpression("lam1");
    lam->captureList = {"ctx"};
    auto lamParam = new Parameter();
    lamParam->id = "lp1";
    lamParam->name = "item";
    lam->addChild("parameters", lamParam);

    auto exprStmt = new ExpressionStatement();
    exprStmt->id = "es1";
    exprStmt->setChild("expression", lam);
    fn->addChild("body", exprStmt);
    mod->addChild("functions", fn);

    json j = toJson(mod.get());
    ASTNode* restored = fromJson(j);

    auto funcs = restored->getChildren("functions");
    CHECK(funcs.size() == 1, "1 function");
    auto body = funcs[0]->getChildren("body");
    CHECK(!body.empty(), "body not empty");

    // Walk down to find the lambda
    auto* es = body[0];
    auto* resLam = es->getChild("expression");
    CHECK(resLam != nullptr, "expression child exists");
    CHECK(resLam->conceptType == "LambdaExpression", "is lambda");
    auto* rl = static_cast<LambdaExpression*>(resLam);
    CHECK(rl->captureList.size() == 1, "capture count");
    CHECK(rl->captureList[0] == "ctx", "capture name");
    delete restored;
    PASS();
}

int main() {
    std::cout << "=== Step 303: New AST Nodes — Async/Lambda/Decorator ===\n";
    test_async_function_construction();
    test_async_function_children();
    test_await_expression();
    test_lambda_construction();
    test_lambda_children();
    test_decorator_construction();
    test_async_json_roundtrip();
    test_await_json_roundtrip();
    test_lambda_json_roundtrip();
    test_decorator_json_roundtrip();
    test_async_with_await_in_module();
    test_lambda_in_function();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
