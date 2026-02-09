// Step 126 TDD Test: Build system detection + error parsing
#include "BuildSystem.h"
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

    // Parse GCC/Clang style
    std::string gcc = "src/main.cpp:12:5: error: something bad";
    auto errs = BuildSystem::parseErrors(gcc);
    expect(errs.size() == 1, "parse gcc error", passed, failed);
    if (!errs.empty()) {
        expect(errs[0].file == "src/main.cpp", "gcc file", passed, failed);
        expect(errs[0].line == 12, "gcc line", passed, failed);
        expect(errs[0].col == 5, "gcc col", passed, failed);
    }

    // Parse MSVC style
    std::string msvc = "C:/proj/main.cpp(42): error C2143: syntax error";
    auto errs2 = BuildSystem::parseErrors(msvc);
    expect(errs2.size() == 1, "parse msvc error", passed, failed);
    if (!errs2.empty()) {
        expect(errs2[0].file.find("main.cpp") != std::string::npos, "msvc file", passed, failed);
        expect(errs2[0].line == 42, "msvc line", passed, failed);
    }

    std::cout << "\n=== Step 126 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
