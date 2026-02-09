// Step 122 TDD Test: Terminal panel command building + output append
#include "TerminalPanel.h"
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

    // Test 1: command line includes cwd + command + stderr redirect
    std::string cmd = TerminalPanel::buildCommandLine("C:\\work", "echo hi");
    expect(cmd.find("echo hi") != std::string::npos, "command contains payload", passed, failed);
    expect(cmd.find("2>&1") != std::string::npos, "command captures stderr", passed, failed);

    // Test 2: output append works
    TerminalPanel panel;
    panel.append("hello");
    panel.append(" world");
    expect(panel.getOutput() == "hello world", "append output", passed, failed);

    std::cout << "\n=== Step 122 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
