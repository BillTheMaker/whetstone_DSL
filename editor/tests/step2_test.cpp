#include "../src/ast/Module.h"
#include "../src/ast/Function.h"
#include "../src/ast/Variable.h"
#include <cassert>
#include <iostream>

int main() {
    // Build tree: Module -> [Function, Function, Variable]
    Module mod("M001", "Calculator", "python");
    Function add("F001", "add");
    Function subtract("F002", "subtract");
    Variable pi("V001", "pi");

    mod.addChild("functions", &add);
    mod.addChild("functions", &subtract);
    mod.addChild("variables", &pi);

    // Verify parent pointers
    assert(add.parent == &mod);
    assert(subtract.parent == &mod);
    assert(pi.parent == &mod);
    assert(mod.parent == nullptr);

    // Verify children by role
    const auto& functions = mod.getChildren("functions");
    assert(functions.size() == 2);
    assert(functions[0] == &add);
    assert(functions[1] == &subtract);

    const auto& variables = mod.getChildren("variables");
    assert(variables.size() == 1);
    assert(variables[0] == &pi);

    // Empty role returns empty vector
    const auto& empty = mod.getChildren("annotations");
    assert(empty.empty());

    // Walk down and back up
    for (auto* child : mod.getChildren("functions")) {
        assert(child->parent == &mod);
        assert(child->parent->conceptType == "Module");
        auto* fn = static_cast<Function*>(child);
        assert(!fn->name.empty());
    }

    // Verify names through downcast
    assert(static_cast<Function*>(functions[0])->name == "add");
    assert(static_cast<Function*>(functions[1])->name == "subtract");
    assert(static_cast<Variable*>(variables[0])->name == "pi");

    // allChildren returns everything
    auto all = mod.allChildren();
    assert(all.size() == 3);

    // Single-valued child (setChild / getChild)
    Function entry("F003", "main");
    mod.setChild("entryPoint", &entry);
    assert(mod.getChild("entryPoint") == &entry);
    assert(entry.parent == &mod);

    // Replace single-valued child
    Function alt("F004", "alt_main");
    mod.setChild("entryPoint", &alt);
    assert(mod.getChild("entryPoint") == &alt);
    assert(alt.parent == &mod);
    assert(entry.parent == nullptr);  // old child's parent cleared

    // getChild on unset role returns nullptr
    assert(mod.getChild("nonexistent") == nullptr);

    std::cout << "Step 2: PASS" << std::endl;
    return 0;
}
