// Step 123 TDD Test: Run/build command selection
#include "EditorUtils.h"
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

    EditorState state;
    std::string py = state.buildRunCommand("C:\\work\\script.py", "python", false);
    expect(py.find("python") != std::string::npos, "python runner", passed, failed);
    expect(py.find("script.py") != std::string::npos, "python path", passed, failed);

    std::string cppRun = state.buildRunCommand("C:\\work\\main.cpp", "cpp", false);
    expect(cppRun.find("g++") != std::string::npos, "cpp compiler", passed, failed);
    expect(cppRun.find("&&") != std::string::npos, "cpp run chain", passed, failed);
#ifdef _WIN32
    expect(cppRun.find(".exe") != std::string::npos, "cpp exe suffix", passed, failed);
#endif

    std::string cppBuild = state.buildRunCommand("C:\\work\\main.cpp", "cpp", true);
    expect(cppBuild.find("&&") == std::string::npos, "cpp build only", passed, failed);

    std::string none = state.buildRunCommand("C:\\work\\main.unknown", "unknown", false);
    expect(none.empty(), "unsupported language", passed, failed);

    std::cout << "\n=== Step 123 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
