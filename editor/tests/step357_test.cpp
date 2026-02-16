// Step 357: Status Bar (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "panels/StatusBar.h"

int main() {
    int passed = 0;
    WhetstoneTheme theme = getDefaultDarkTheme();

    // Test 1: language badge shows current buffer language
    {
        StatusBarInput in;
        in.language = "python";
        in.filePath = "src/main.py";
        auto model = buildStatusBar(in, theme);
        assert(model.languageBadge == "python");
        std::cout << "Test 1 PASSED: language badge\n";
        passed++;
    }

    // Test 2: cursor position updates
    {
        StatusBarInput in;
        in.filePath = "file";
        in.line = 42;
        in.column = 8;
        auto model = buildStatusBar(in, theme);
        assert(model.centerText.find("Ln 42, Col 8") != std::string::npos);
        std::cout << "Test 2 PASSED: cursor position\n";
        passed++;
    }

    // Test 3: diagnostic count accurate
    {
        StatusBarInput in;
        in.filePath = "file";
        in.errorCount = 3;
        in.warningCount = 2;
        auto model = buildStatusBar(in, theme);
        assert(model.diagnosticsText == "3 errors, 2 warnings");
        std::cout << "Test 3 PASSED: diagnostics count\n";
        passed++;
    }

    // Test 4: workflow badge shown when active
    {
        StatusBarInput in;
        in.filePath = "file";
        in.workflowActive = true;
        in.workflowCurrent = 3;
        in.workflowTotal = 10;
        auto model = buildStatusBar(in, theme);
        assert(model.workflowBadge == "Executing 3/10");
        std::cout << "Test 4 PASSED: workflow badge active\n";
        passed++;
    }

    // Test 5: click diagnostics focuses diagnostics panel
    {
        assert(statusBarClickDiagnosticsTarget() == "diagnostics");
        std::cout << "Test 5 PASSED: diagnostics click target\n";
        passed++;
    }

    // Test 6: status bar uses theme colors
    {
        StatusBarInput in;
        in.filePath = "file";
        auto model = buildStatusBar(in, theme);
        assert(model.bgColor.toHex() == theme.bgPanel.toHex());
        assert(model.textColor.toHex() == theme.text.toHex());
        assert(model.dimTextColor.toHex() == theme.textDim.toHex());
        std::cout << "Test 6 PASSED: theme colors\n";
        passed++;
    }

    // Test 7: language changes when switching buffers
    {
        StatusBarInput py;
        py.language = "python";
        py.filePath = "a.py";
        StatusBarInput cpp;
        cpp.language = "cpp";
        cpp.filePath = "a.cpp";
        auto m1 = buildStatusBar(py, theme);
        auto m2 = buildStatusBar(cpp, theme);
        assert(m1.languageBadge != m2.languageBadge);
        std::cout << "Test 7 PASSED: language switch reflected\n";
        passed++;
    }

    // Test 8: encoding display
    {
        StatusBarInput in;
        in.filePath = "file";
        in.encoding = "UTF-16";
        auto model = buildStatusBar(in, theme);
        assert(model.centerText.find("UTF-16") != std::string::npos);
        std::cout << "Test 8 PASSED: encoding display\n";
        passed++;
    }

    // Test 9: all sections fit without overflow for normal input
    {
        StatusBarInput in;
        in.filePath = "src/main.cpp";
        auto model = buildStatusBar(in, theme, 200);
        assert(!model.overflow);
        std::cout << "Test 9 PASSED: no overflow normal case\n";
        passed++;
    }

    // Test 10: overflow flagged for extreme long path
    {
        StatusBarInput in;
        in.filePath = std::string(220, 'a');
        auto model = buildStatusBar(in, theme, 80);
        assert(model.overflow);
        std::cout << "Test 10 PASSED: overflow flagged\n";
        passed++;
    }

    // Test 11: line ending display
    {
        StatusBarInput in;
        in.filePath = "file";
        in.lineEnding = "CRLF";
        auto model = buildStatusBar(in, theme);
        assert(model.centerText.find("CRLF") != std::string::npos);
        std::cout << "Test 11 PASSED: line ending display\n";
        passed++;
    }

    // Test 12: MCP indicator states
    {
        StatusBarInput on;
        on.filePath = "file";
        on.mcpConnected = true;
        StatusBarInput off = on;
        off.mcpConnected = false;
        auto a = buildStatusBar(on, theme);
        auto b = buildStatusBar(off, theme);
        assert(a.mcpText == "MCP: connected");
        assert(b.mcpText == "MCP: offline");
        std::cout << "Test 12 PASSED: MCP indicator states\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
