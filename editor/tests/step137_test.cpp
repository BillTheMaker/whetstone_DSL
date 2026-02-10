// Step 137 TDD Test: Composition builder
#include "CompositionBuilder.h"
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

    std::vector<PrimitiveSymbol> prims = {
        {"normalize", "function", "import", "lib"},
        {"tokenize", "function", "import", "lib"}
    };
    auto steps = suggestCompositionSteps(prims, "string", "string");
    expect(!steps.empty(), "suggest steps", passed, failed);

    std::vector<CompositionStep> pipeline = {
        {"normalize", "string", "string"},
        {"tokenize", "string", "list"}
    };
    std::string code = buildCompositionCode(pipeline, "input");
    expect(code == "tokenize(normalize(input))", "compose nested calls", passed, failed);

    std::string func = buildCompositionFunction(pipeline, "pipe", "input");
    expect(func.find("def pipe") != std::string::npos, "function header", passed, failed);
    expect(func.find("return tokenize(normalize(input))") != std::string::npos,
           "function return", passed, failed);

    std::cout << "\n=== Step 137 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
