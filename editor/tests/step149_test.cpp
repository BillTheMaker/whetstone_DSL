// Step 149 TDD Test: Java CST-to-AST + generator
#include "ast/Parser.h"
#include "ast/Generator.h"
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

int main() {
    int passed = 0;
    int failed = 0;

    std::string java = R"(
import java.util.List;
public class Box {
  @Deprecated
  public int size(int n) { return n + 1; }
  public Box() { }
}
)";

    auto mod = TreeSitterParser::parseJava(java);
    expect(mod != nullptr, "java module created", passed, failed);
    auto funcs = mod->getChildren("functions");
    expect(funcs.size() >= 1, "java functions discovered", passed, failed);

    bool foundSize = false;
    bool foundIntReturn = false;
    for (const auto* fnNode : funcs) {
        if (fnNode->conceptType != "Function") continue;
        auto* fn = static_cast<const Function*>(fnNode);
        if (fn->name == "Box.size") {
            foundSize = true;
            auto* retType = fn->getChild("returnType");
            if (retType && retType->conceptType == "PrimitiveType") {
                auto* prim = static_cast<const PrimitiveType*>(retType);
                foundIntReturn = (prim->kind == "int");
            }
        }
    }
    expect(foundSize, "java method named Box.size", passed, failed);
    expect(foundIntReturn, "java return type parsed", passed, failed);

    JavaGenerator gen;
    std::string out = gen.generate(mod.get());
    expect(out.find("import java.util.List;") != std::string::npos,
           "java imports generated", passed, failed);
    expect(out.find("class Box") != std::string::npos,
           "java class generated", passed, failed);
    expect(out.find("int size") != std::string::npos,
           "java method signature generated", passed, failed);
    expect(out.find("Box(") != std::string::npos,
           "java constructor generated", passed, failed);

    std::cout << "\n=== Step 149 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
