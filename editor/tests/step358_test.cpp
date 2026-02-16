// Step 358: Go-to-Definition + Go-to-Symbol (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "panels/NavigationTools.h"
#include "ast/ClassDeclaration.h"
#include "ast/Function.h"
#include "ast/Variable.h"

int main() {
    int passed = 0;

    // Build module A
    Module modA("mA", "modA", "python");
    auto* fnHelperA = new Function("f1", "helper");
    fnHelperA->setSpan(9, 0, 12, 0);  // line 10 human-readable
    auto* fnCaller = new Function("f2", "caller");
    fnCaller->setSpan(29, 0, 33, 0);
    auto* cls = new ClassDeclaration("c1", "Engine");
    cls->setSpan(19, 0, 24, 0);
    auto* var = new Variable("v1", "count");
    var->setSpan(14, 0, 14, 5);
    modA.addChild("functions", fnHelperA);
    modA.addChild("functions", fnCaller);
    modA.addChild("classes", cls);
    modA.addChild("variables", var);

    // Build module B with cross-file helper
    Module modB("mB", "modB", "python");
    auto* fnHelperB = new Function("f3", "external_helper");
    fnHelperB->setSpan(4, 0, 8, 0);
    modB.addChild("functions", fnHelperB);

    std::vector<std::pair<std::string, const Module*>> modules = {
        {"a.py", &modA},
        {"b.py", &modB}
    };

    // Test 1: F12 on function call jumps to definition
    {
        FunctionCall call;
        call.functionName = "helper";
        auto def = goToDefinition(&call, "a.py", modules);
        assert(def.has_value());
        assert(def->name == "helper");
        assert(def->filePath == "a.py");
        std::cout << "Test 1 PASSED: F12 go-to-definition\n";
        passed++;
    }

    // Test 2: Ctrl+Shift+O lists file symbols
    {
        auto symbols = collectFileSymbols(&modA, "a.py");
        assert(symbols.size() >= 4);
        std::cout << "Test 2 PASSED: file symbol list\n";
        passed++;
    }

    // Test 3: Ctrl+T lists project symbols
    {
        auto symbols = collectProjectSymbols(modules);
        assert(symbols.size() >= 5);
        bool hasFromB = false;
        for (const auto& s : symbols) {
            if (s.filePath == "b.py" && s.name == "external_helper") hasFromB = true;
        }
        assert(hasFromB);
        std::cout << "Test 3 PASSED: project symbol list\n";
        passed++;
    }

    // Test 4: symbol list shows icons by type
    {
        auto symbols = collectFileSymbols(&modA, "a.py");
        bool classIcon = false;
        bool fnIcon = false;
        for (const auto& s : symbols) {
            if (s.name == "Engine" && !s.icon.empty()) classIcon = true;
            if (s.name == "helper" && !s.icon.empty()) fnIcon = true;
        }
        assert(classIcon && fnIcon);
        std::cout << "Test 4 PASSED: symbols include icons\n";
        passed++;
    }

    // Test 5: filter works on symbol list
    {
        auto symbols = collectFileSymbols(&modA, "a.py");
        auto filtered = filterSymbols(symbols, "eng");
        assert(filtered.size() == 1);
        assert(filtered[0].name == "Engine");
        std::cout << "Test 5 PASSED: symbol filter\n";
        passed++;
    }

    // Test 6: cross-file jump opens target buffer path
    {
        FunctionCall call;
        call.functionName = "external_helper";
        auto def = goToDefinition(&call, "a.py", modules);
        assert(def.has_value());
        assert(def->filePath == "b.py");
        std::cout << "Test 6 PASSED: cross-file jump\n";
        passed++;
    }

    // Test 7: no match shows message
    {
        FunctionCall call;
        call.functionName = "missing_fn";
        auto def = goToDefinition(&call, "a.py", modules);
        assert(!def.has_value());
        assert(definitionStatusMessage(def) == "No definition found");
        std::cout << "Test 7 PASSED: no-match message\n";
        passed++;
    }

    // Test 8: symbols sorted by line number
    {
        auto symbols = collectFileSymbols(&modA, "a.py");
        int prev = -1;
        for (const auto& s : symbols) {
            if (s.line < 0) continue;
            assert(s.line >= prev);
            prev = s.line;
        }
        std::cout << "Test 8 PASSED: sorted by line\n";
        passed++;
    }

    // Test 9: empty query returns all
    {
        auto symbols = collectProjectSymbols(modules);
        auto filtered = filterSymbols(symbols, "");
        assert(filtered.size() == symbols.size());
        std::cout << "Test 9 PASSED: empty query returns all\n";
        passed++;
    }

    // Test 10: case-insensitive filter
    {
        auto symbols = collectProjectSymbols(modules);
        auto filtered = filterSymbols(symbols, "EXTERNAL");
        assert(filtered.size() == 1);
        assert(filtered[0].name == "external_helper");
        std::cout << "Test 10 PASSED: case-insensitive filter\n";
        passed++;
    }

    // Test 11: definition prefers current file over project
    {
        // Add same-name function in modB to validate preference.
        auto* shadow = new Function("f4", "helper");
        shadow->setSpan(1, 0, 2, 0);
        modB.addChild("functions", shadow);

        FunctionCall call;
        call.functionName = "helper";
        auto def = goToDefinition(&call, "a.py", modules);
        assert(def.has_value());
        assert(def->filePath == "a.py");
        std::cout << "Test 11 PASSED: current-file preference\n";
        passed++;
    }

    // Test 12: null call safely handled
    {
        auto def = goToDefinition(nullptr, "a.py", modules);
        assert(!def.has_value());
        std::cout << "Test 12 PASSED: null call safe\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
