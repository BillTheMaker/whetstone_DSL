// Step 365: WAT Parser (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "ast/WatParser.h"
#include "Pipeline.h"

static bool hasBodyNode(const Function* fn, const std::string& conceptType) {
    for (auto* n : fn->getChildren("body")) {
        if (n->conceptType == conceptType) return true;
    }
    return false;
}

int main() {
    int passed = 0;

    // Test 1: module parsing
    {
        std::string src = "(module (func $add (param $x i32) (param $y i32) (result i32)))";
        auto mod = WatParser::parseWat(src);
        assert(mod != nullptr);
        assert(mod->targetLanguage == "wat");
        std::cout << "Test 1 PASSED: module parse\n";
        passed++;
    }

    // Test 2: function with params/result
    {
        std::string src = "(module (func $add (param $x i32) (param $y i32) (result i32)))";
        auto mod = WatParser::parseWat(src);
        assert(mod->getChildren("functions").size() == 1);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(fn->name == "$add");
        assert(fn->getChildren("parameters").size() == 2);
        auto* ret = fn->getChild("returnType");
        assert(ret != nullptr && ret->conceptType == "PrimitiveType");
        std::cout << "Test 2 PASSED: function params/result\n";
        passed++;
    }

    // Test 3: local variables
    {
        std::string src = "(module (func $f (local $tmp i32) (result i32)))";
        auto mod = WatParser::parseWat(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "Variable"));
        std::cout << "Test 3 PASSED: local variable parse\n";
        passed++;
    }

    // Test 4: export declarations
    {
        std::string src = "(module (func $f) (export \"f\" (func $f)))";
        auto mod = WatParser::parseWat(src);
        bool hasExportPragma = false;
        for (auto* s : mod->getChildren("statements")) {
            if (s->conceptType == "PragmaDirective") hasExportPragma = true;
        }
        assert(hasExportPragma);
        std::cout << "Test 4 PASSED: export declaration parse\n";
        passed++;
    }

    // Test 5: import declarations
    {
        std::string src = "(module (import \"env\" \"print\" (func $print (param $x i32))))";
        auto mod = WatParser::parseWat(src);
        assert(!mod->getChildren("imports").empty());
        std::cout << "Test 5 PASSED: import declaration parse\n";
        passed++;
    }

    // Test 6: global variables
    {
        std::string src = "(module (global $g (mut i32) (i32.const 0)))";
        auto mod = WatParser::parseWat(src);
        assert(!mod->getChildren("variables").empty());
        std::cout << "Test 6 PASSED: global variable parse\n";
        passed++;
    }

    // Test 7: i32.add -> BinaryOperation
    {
        std::string src = "(module (func $add (param $x i32) (param $y i32) (result i32) local.get $x local.get $y i32.add))";
        auto mod = WatParser::parseWat(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "BinaryOperation"));
        std::cout << "Test 7 PASSED: i32.add mapped to BinaryOperation\n";
        passed++;
    }

    // Test 8: call -> FunctionCall
    {
        std::string src = "(module (func $f call $helper) (func $helper))";
        auto mod = WatParser::parseWat(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "FunctionCall"));
        std::cout << "Test 8 PASSED: call mapped to FunctionCall\n";
        passed++;
    }

    // Test 9: if -> IfStatement
    {
        std::string src = "(module (func $f (if (result i32) (then (i32.const 1)) (else (i32.const 0)))))";
        auto mod = WatParser::parseWat(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "IfStatement"));
        std::cout << "Test 9 PASSED: if mapped to IfStatement\n";
        passed++;
    }

    // Test 10: nested block/loop keeps structural block node
    {
        std::string src = "(module (func $f (block (loop (br 0)))))";
        auto mod = WatParser::parseWat(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "Block"));
        std::cout << "Test 10 PASSED: block/loop structural mapping\n";
        passed++;
    }

    // Test 11: empty module
    {
        std::string src = "(module)";
        auto mod = WatParser::parseWat(src);
        assert(mod != nullptr);
        assert(mod->getChildren("functions").empty());
        std::cout << "Test 11 PASSED: empty module parse\n";
        passed++;
    }

    // Test 12: wat/wasm pipeline route and type preservation
    {
        std::string src = "(module (func $f (param $x i64) (result f64)))";
        Pipeline p;
        std::vector<ParseDiagnostic> diags;
        auto mod = p.parse(src, "wat", diags);
        assert(mod != nullptr);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        auto* param = static_cast<Parameter*>(fn->getChildren("parameters")[0]);
        auto* pType = static_cast<PrimitiveType*>(param->getChild("type"));
        auto* rType = static_cast<PrimitiveType*>(fn->getChild("returnType"));
        assert(pType->kind == "i64");
        assert(rType->kind == "f64");

        auto mod2 = p.parse(src, "wasm", diags);
        assert(mod2 != nullptr);
        std::cout << "Test 12 PASSED: pipeline wat/wasm routing + type preservation\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
