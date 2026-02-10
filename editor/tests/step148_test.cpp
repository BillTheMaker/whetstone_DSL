// Step 148 TDD Test: JavaScript/TypeScript generator
#include "ast/Generator.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Import.h"
#include "ast/Annotation.h"
#include "ast/Type.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

static Function* makeAddFunction() {
    auto* fn = new Function("fn_add", "add");
    auto* a = new Parameter("param_a", "a");
    auto* b = new Parameter("param_b", "b");
    fn->addChild("parameters", a);
    fn->addChild("parameters", b);

    auto* binOp = new BinaryOperation("bin_add", "+");
    binOp->setChild("left", new VariableReference("var_a", "a"));
    binOp->setChild("right", new VariableReference("var_b", "b"));

    auto* ret = new Return();
    ret->id = "ret_add";
    ret->setChild("value", binOp);
    fn->addChild("body", ret);
    return fn;
}

static Function* makeClassMethod() {
    auto* fn = new Function("fn_size", "Box.size");
    auto* n = new Parameter("param_n", "n");
    fn->addChild("parameters", n);

    auto* ret = new Return();
    ret->id = "ret_size";
    ret->setChild("value", new VariableReference("var_n", "n"));
    fn->addChild("body", ret);
    return fn;
}

int main() {
    int passed = 0;
    int failed = 0;

    Module jsMod;
    jsMod.id = "mod_js";
    jsMod.name = "App";
    jsMod.targetLanguage = "javascript";

    auto* imp = new Import("imp_react", "react", "module", "React");
    jsMod.addChild("imports", imp);

    auto* reclaim = new ReclaimAnnotation("anno_reclaim", "Tracing");
    jsMod.addChild("annotations", reclaim);

    jsMod.addChild("functions", makeAddFunction());
    jsMod.addChild("functions", makeClassMethod());

    JavaScriptGenerator jsGen;
    std::string jsOut = jsGen.generate(&jsMod);

    expect(jsOut.find("import * as React from 'react';") != std::string::npos,
           "js imports emitted", passed, failed);
    expect(jsOut.find("function add") != std::string::npos,
           "js function emitted", passed, failed);
    expect(jsOut.find("class Box") != std::string::npos,
           "js class emitted", passed, failed);
    expect(jsOut.find("FinalizationRegistry") != std::string::npos,
           "js reclaim mapping emitted", passed, failed);

    Module tsMod;
    tsMod.id = "mod_ts";
    tsMod.name = "App";
    tsMod.targetLanguage = "typescript";

    auto* greet = new Function("fn_greet", "greet");
    auto* nameParam = new Parameter("param_name", "name");
    auto* nameType = new PrimitiveType("type_name", "string");
    nameParam->setChild("type", nameType);
    greet->addChild("parameters", nameParam);

    auto* retType = new PrimitiveType("type_ret", "string");
    greet->setChild("returnType", retType);

    auto* ret = new Return();
    ret->id = "ret_greet";
    ret->setChild("value", new VariableReference("var_name", "name"));
    greet->addChild("body", ret);
    tsMod.addChild("functions", greet);

    auto* countVar = new Variable("var_count", "count");
    auto* countType = new PrimitiveType("type_count", "int");
    countVar->setChild("type", countType);
    countVar->setChild("initializer", new IntegerLiteral("int_one", 1));
    tsMod.addChild("variables", countVar);

    TypeScriptGenerator tsGen;
    std::string tsOut = tsGen.generate(&tsMod);

    expect(tsOut.find("function greet(name: string): string") != std::string::npos,
           "ts types in function signature", passed, failed);
    expect(tsOut.find("let count: number = 1;") != std::string::npos,
           "ts types in variable", passed, failed);

    std::cout << "\n=== Step 148 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
