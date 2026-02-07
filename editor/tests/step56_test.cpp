// Step 56 TDD Test: Robust Emacs command integration
//
// Tests that the Emacs integration handles commands properly:
// 1. Elisp command builder escapes special characters
// 2. Emacs commands trigger orchestrator RPCs (loadFile, saveFile)
// 3. Error handling: detect daemon crash
// 4. Auto-restart on daemon failure
// 5. Command builder produces valid Elisp strings
//
// Will fail until the robust command integration is implemented.

#include <iostream>
#include <string>
#include <cassert>

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// Forward declaration — ElispCommandBuilder that doesn't exist yet
class ElispCommandBuilder {
public:
    // Build an Elisp expression to open a file
    static std::string findFile(const std::string& path);

    // Build an Elisp expression to save current buffer
    static std::string saveBuffer();

    // Build an Elisp expression to evaluate arbitrary Elisp
    static std::string eval(const std::string& elisp);

    // Escape special characters in strings for Elisp embedding
    static std::string escapeString(const std::string& str);
};

// Forward declaration — EmacsConnection for daemon management
class EmacsConnection {
public:
    // Start Emacs daemon
    bool startDaemon();

    // Check if daemon is running
    bool isDaemonAlive() const;

    // Send command to Emacs and get result
    std::string sendCommand(const std::string& elispCommand);

    // Auto-restart daemon if it died
    bool ensureDaemon();

    // Get the last error (if sendCommand failed)
    std::string getLastError() const;
};

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
        // Should be wrapped in parentheses for valid Elisp
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
        // The path should be inside a string literal
        assert(contains(cmd, "\"") && "Path should be in a string literal");

        std::cout << "Test 4 PASS: Paths with spaces handled correctly" << std::endl;
        ++passed;
    }

    // --- Test 5: EmacsConnection.ensureDaemon starts daemon if not running ---
    {
        EmacsConnection conn;
        // Initially daemon may not be running
        bool result = conn.ensureDaemon();
        assert(result && "ensureDaemon should succeed (start daemon if needed)");
        assert(conn.isDaemonAlive() && "Daemon should be alive after ensureDaemon");

        std::cout << "Test 5 PASS: ensureDaemon starts daemon if needed" << std::endl;
        ++passed;
    }

    // --- Test 6: sendCommand returns error on invalid Elisp ---
    {
        EmacsConnection conn;
        conn.ensureDaemon();

        std::string result = conn.sendCommand("(this-is-not-a-real-function)");
        std::string error = conn.getLastError();
        // Should have some error indication (either in result or error string)
        assert(!error.empty() || contains(result, "error") || contains(result, "void") &&
               "Invalid command should produce an error");

        std::cout << "Test 6 PASS: Invalid Elisp returns error" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 56 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
