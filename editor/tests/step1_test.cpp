#include "../src/ast/Module.h"
#include <cassert>
#include <iostream>

int main() {
    Module m("SFE_M001", "SimpleFunctionExample", "python");

    assert(m.id == "SFE_M001");
    assert(m.name == "SimpleFunctionExample");
    assert(m.targetLanguage == "python");
    assert(m.conceptType == "Module");
    assert(m.parent == nullptr);

    std::cout << "Step 1: PASS" << std::endl;
    return 0;
}
