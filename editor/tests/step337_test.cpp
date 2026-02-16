// Step 337: Preprocessor AST Nodes (12 tests)
// Tests IncludeDirective, PragmaDirective, MacroDefinition construction,
// serialization roundtrip, and CompactAST output.

#include <cassert>
#include <iostream>
#include <string>
#include "ast/PreprocessorNodes.h"
#include "ast/Serialization.h"
#include "CompactAST.h"
#include "ast/Module.h"
#include "ast/Function.h"

int main() {
    int passed = 0;

    // Test 1: IncludeDirective construction — local include
    {
        auto* inc = new IncludeDirective("inc1", "ast/ASTNode.h", false);
        assert(inc->conceptType == "IncludeDirective");
        assert(inc->path == "ast/ASTNode.h");
        assert(!inc->isSystem);
        delete inc;
        std::cout << "Test 1 PASSED: IncludeDirective local include\n";
        passed++;
    }

    // Test 2: IncludeDirective — system include
    {
        auto* inc = new IncludeDirective("inc2", "string", true);
        assert(inc->isSystem);
        assert(inc->path == "string");
        delete inc;
        std::cout << "Test 2 PASSED: IncludeDirective system include\n";
        passed++;
    }

    // Test 3: IncludeDirective JSON roundtrip
    {
        auto* inc = new IncludeDirective("inc3", "vector", true);
        json j = toJson(inc);
        auto* restored = fromJson(j);
        assert(restored->conceptType == "IncludeDirective");
        auto* ri = static_cast<IncludeDirective*>(restored);
        assert(ri->path == "vector");
        assert(ri->isSystem == true);
        delete inc;
        delete restored;
        std::cout << "Test 3 PASSED: IncludeDirective JSON roundtrip\n";
        passed++;
    }

    // Test 4: PragmaDirective construction
    {
        auto* prag = new PragmaDirective("prag1", "once");
        assert(prag->conceptType == "PragmaDirective");
        assert(prag->directive == "once");
        delete prag;
        std::cout << "Test 4 PASSED: PragmaDirective construction\n";
        passed++;
    }

    // Test 5: PragmaDirective JSON roundtrip
    {
        auto* prag = new PragmaDirective("prag2", "pack(push, 1)");
        json j = toJson(prag);
        auto* restored = fromJson(j);
        assert(restored->conceptType == "PragmaDirective");
        auto* rp = static_cast<PragmaDirective*>(restored);
        assert(rp->directive == "pack(push, 1)");
        delete prag;
        delete restored;
        std::cout << "Test 5 PASSED: PragmaDirective JSON roundtrip\n";
        passed++;
    }

    // Test 6: MacroDefinition — object-like macro
    {
        auto* mac = new MacroDefinition("mac1", "MAX_SIZE");
        mac->body = "1024";
        mac->isFunctionLike = false;
        assert(mac->conceptType == "MacroDefinition");
        assert(mac->name == "MAX_SIZE");
        assert(mac->body == "1024");
        assert(!mac->isFunctionLike);
        assert(mac->parameters.empty());
        delete mac;
        std::cout << "Test 6 PASSED: MacroDefinition object-like macro\n";
        passed++;
    }

    // Test 7: MacroDefinition — function-like macro with params
    {
        auto* mac = new MacroDefinition("mac2", "MIN");
        mac->isFunctionLike = true;
        mac->parameters = {"a", "b"};
        mac->body = "((a) < (b) ? (a) : (b))";
        assert(mac->isFunctionLike);
        assert(mac->parameters.size() == 2);
        assert(mac->parameters[0] == "a");
        assert(mac->parameters[1] == "b");
        delete mac;
        std::cout << "Test 7 PASSED: MacroDefinition function-like macro\n";
        passed++;
    }

    // Test 8: MacroDefinition JSON roundtrip
    {
        auto* mac = new MacroDefinition("mac3", "CONCAT");
        mac->isFunctionLike = true;
        mac->parameters = {"x", "y"};
        mac->body = "x##y";
        json j = toJson(mac);
        auto* restored = fromJson(j);
        assert(restored->conceptType == "MacroDefinition");
        auto* rm = static_cast<MacroDefinition*>(restored);
        assert(rm->name == "CONCAT");
        assert(rm->isFunctionLike);
        assert(rm->parameters.size() == 2);
        assert(rm->parameters[0] == "x");
        assert(rm->body == "x##y");
        delete mac;
        delete restored;
        std::cout << "Test 8 PASSED: MacroDefinition JSON roundtrip\n";
        passed++;
    }

    // Test 9: CompactAST names
    {
        auto* inc = new IncludeDirective("inc4", "iostream", true);
        assert(getNodeName(inc) == "iostream");
        auto* prag = new PragmaDirective("prag3", "once");
        assert(getNodeName(prag) == "once");
        auto* mac = new MacroDefinition("mac4", "DEBUG_LOG");
        assert(getNodeName(mac) == "DEBUG_LOG");
        delete inc; delete prag; delete mac;
        std::cout << "Test 9 PASSED: CompactAST node names\n";
        passed++;
    }

    // Test 10: Module with mixed preprocessor + function nodes
    {
        auto* mod = new Module();
        mod->id = "mod1";
        mod->name = "test_module";
        auto* inc = new IncludeDirective("inc5", "stdio.h", true);
        auto* prag = new PragmaDirective("prag4", "once");
        auto* mac = new MacroDefinition("mac5", "VERSION");
        mac->body = "\"1.0\"";
        auto* func = new Function("fn1", "main");
        mod->addChild("statements", inc);
        mod->addChild("statements", prag);
        mod->addChild("statements", mac);
        mod->addChild("functions", func);

        auto stmts = mod->getChildren("statements");
        assert(stmts.size() == 3);
        assert(stmts[0]->conceptType == "IncludeDirective");
        assert(stmts[1]->conceptType == "PragmaDirective");
        assert(stmts[2]->conceptType == "MacroDefinition");
        delete mod;
        std::cout << "Test 10 PASSED: Module with mixed preprocessor + function\n";
        passed++;
    }

    // Test 11: Empty macro body
    {
        auto* mac = new MacroDefinition("mac6", "NDEBUG");
        assert(mac->body.empty());
        json j = toJson(mac);
        auto* restored = fromJson(j);
        auto* rm = static_cast<MacroDefinition*>(restored);
        assert(rm->name == "NDEBUG");
        assert(rm->body.empty());
        assert(!rm->isFunctionLike);
        delete mac;
        delete restored;
        std::cout << "Test 11 PASSED: Empty macro body\n";
        passed++;
    }

    // Test 12: Serialization createNode dispatch for all 3 types
    {
        json jInc = {{"concept", "IncludeDirective"}, {"id", "test_inc"},
                     {"properties", {{"path", "test.h"}, {"isSystem", false}}}};
        auto* rInc = fromJson(jInc);
        assert(rInc != nullptr);
        assert(rInc->conceptType == "IncludeDirective");
        assert(static_cast<IncludeDirective*>(rInc)->path == "test.h");

        json jPrag = {{"concept", "PragmaDirective"}, {"id", "test_prag"},
                      {"properties", {{"directive", "warning disable"}}}};
        auto* rPrag = fromJson(jPrag);
        assert(rPrag != nullptr);
        assert(static_cast<PragmaDirective*>(rPrag)->directive == "warning disable");

        json jMac = {{"concept", "MacroDefinition"}, {"id", "test_mac"},
                     {"properties", {{"name", "FOO"}, {"body", "bar"}}}};
        auto* rMac = fromJson(jMac);
        assert(rMac != nullptr);
        assert(static_cast<MacroDefinition*>(rMac)->name == "FOO");

        delete rInc; delete rPrag; delete rMac;
        std::cout << "Test 12 PASSED: Serialization createNode dispatch\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
