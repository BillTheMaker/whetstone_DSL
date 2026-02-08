// Step 56 TDD Test: Robust Emacs command integration
//
// Tests that:
// 1. ElispCommandBuilder escapes special characters
// 2. findFile builds correct Elisp
// 3. saveBuffer builds correct Elisp
// 4. File paths with spaces are properly escaped
// 5. MockEmacsConnection.ensureDaemon succeeds
// 6. sendCommand returns error on unknown function

#include <iostream>
#include <string>
#include <cassert>
#include "EmacsIntegration.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: ElispCommandBuilder escapes special characters ---
    {
        std::string escaped = ElispCommandBuilder::escapeString("path\\with\"quotes");
        assert(contains(escaped, "\\\\") && "Backslashes should be escaped");
        assert(contains(escaped, "\\\"") && "Quotes should be escaped");

        std::cout << "Test 1 PASS: Special characters escaped correctly" << std::endl;
        ++passed;
    }

    // --- Test 2: findFile builds correct Elisp ---
    {
        std::string cmd = ElispCommandBuilder::findFile("/home/user/test.py");
        assert(contains(cmd, "find-file") && "Should use find-file command");
        assert(contains(cmd, "/home/user/test.py") && "Should contain the file path");
        assert(cmd.front() == '(' && "Should start with '('");

        std::cout << "Test 2 PASS: findFile builds valid Elisp" << std::endl;
        ++passed;
    }

    // --- Test 3: saveBuffer builds correct Elisp ---
    {
        std::string cmd = ElispCommandBuilder::saveBuffer();
        assert(contains(cmd, "save-buffer") && "Should use save-buffer command");
        assert(cmd.front() == '(' && "Should start with '('");

        std::cout << "Test 3 PASS: saveBuffer builds valid Elisp" << std::endl;
        ++passed;
    }

    // --- Test 4: File paths with spaces are properly escaped ---
    {
        std::string cmd = ElispCommandBuilder::findFile("/home/user/my project/test.py");
        assert(contains(cmd, "my project") && "Path with spaces should be preserved");
        assert(contains(cmd, "\"") && "Path should be in a string literal");

        std::cout << "Test 4 PASS: Paths with spaces handled correctly" << std::endl;
        ++passed;
    }

    // --- Test 5: MockEmacsConnection.ensureDaemon starts daemon ---
    {
        MockEmacsConnection conn;
        bool result = conn.ensureDaemon();
        assert(result && "ensureDaemon should succeed");
        assert(conn.isDaemonAlive() && "Daemon should be alive after ensureDaemon");

        std::cout << "Test 5 PASS: ensureDaemon starts daemon if needed" << std::endl;
        ++passed;
    }

    // --- Test 6: sendCommand returns error on invalid Elisp ---
    {
        MockEmacsConnection conn;
        conn.ensureDaemon();

        std::string result = conn.sendCommand("(this-is-not-a-real-function)");
        std::string error = conn.getLastError();
        assert((!error.empty() || contains(result, "error") || contains(result, "void")) &&
               "Invalid command should produce an error");

        std::cout << "Test 6 PASS: Invalid Elisp returns error" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 56 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
