// Step 55 TDD Test: Welcome screen component
//
// Verifies that WelcomeScreen:
// 1. Has default title and subtitle
// 2. Has default quick actions
// 3. Recent files can be added and retrieved
// 4. Tips cycle correctly
// 5. Visibility toggle works
// 6. Recent files limit (max 10) and deduplication
// 7. Version info is accessible

#include <iostream>
#include <string>
#include <cassert>
#include "WelcomeScreen.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Default title and subtitle ---
    {
        WelcomeScreen ws;
        assert(ws.getTitle() == "Whetstone Editor" && "Default title");
        assert(!ws.getSubtitle().empty() && "Should have a subtitle");

        std::cout << "Test 1 PASS: Default title and subtitle" << std::endl;
        ++passed;
    }

    // --- Test 2: Default quick actions ---
    {
        WelcomeScreen ws;
        auto actions = ws.getActions();
        assert(actions.size() >= 3 && "Should have at least 3 default actions");

        // Check for expected actions
        bool hasNewFile = false, hasOpenFile = false;
        for (const auto& a : actions) {
            if (a.label == "New File") hasNewFile = true;
            if (a.label == "Open File") hasOpenFile = true;
        }
        assert(hasNewFile && "Should have New File action");
        assert(hasOpenFile && "Should have Open File action");

        std::cout << "Test 2 PASS: Default quick actions present" << std::endl;
        ++passed;
    }

    // --- Test 3: Add and retrieve recent files ---
    {
        WelcomeScreen ws;
        ws.addRecentFile("/home/user/test.py", "python");
        ws.addRecentFile("/home/user/main.cpp", "cpp");

        auto recent = ws.getRecentFiles();
        assert(recent.size() == 2 && "Should have 2 recent files");
        // Most recent first
        assert(recent[0].path == "/home/user/main.cpp" && "Most recent first");
        assert(recent[0].displayName == "main.cpp" && "Display name extracted");
        assert(recent[0].language == "cpp" && "Language preserved");
        assert(recent[1].path == "/home/user/test.py" && "Second file");

        std::cout << "Test 3 PASS: Recent files added and retrieved" << std::endl;
        ++passed;
    }

    // --- Test 4: Tips cycle through ---
    {
        WelcomeScreen ws;
        assert(!ws.getTips().empty() && "Should have default tips");

        std::string tip0 = ws.getNextTip();
        std::string tip1 = ws.getNextTip();
        assert(!tip0.empty() && "First tip should not be empty");
        assert(tip0 != tip1 && "Tips should cycle to different values");

        // Cycle back to first
        size_t numTips = ws.getTips().size();
        for (size_t i = 2; i < numTips; ++i) ws.getNextTip();
        std::string tipCycled = ws.getNextTip();
        assert(tipCycled == tip0 && "Tips should cycle back to first");

        std::cout << "Test 4 PASS: Tips cycle correctly" << std::endl;
        ++passed;
    }

    // --- Test 5: Visibility toggle ---
    {
        WelcomeScreen ws;
        assert(ws.isVisible() && "Should be visible by default");
        ws.hide();
        assert(!ws.isVisible() && "Should be hidden after hide()");
        ws.show();
        assert(ws.isVisible() && "Should be visible after show()");

        std::cout << "Test 5 PASS: Visibility toggle works" << std::endl;
        ++passed;
    }

    // --- Test 6: Recent files max limit and dedup ---
    {
        WelcomeScreen ws;
        // Add 12 files
        for (int i = 0; i < 12; ++i) {
            ws.addRecentFile("/test/file" + std::to_string(i) + ".py", "python");
        }
        auto recent = ws.getRecentFiles();
        assert(recent.size() == 10 && "Should cap at 10 recent files");

        // Add duplicate — should not increase count
        ws.clearRecentFiles();
        ws.addRecentFile("/test/a.py", "python");
        ws.addRecentFile("/test/a.py", "python");
        assert(ws.getRecentFiles().size() == 1 && "Duplicate should not be added");

        std::cout << "Test 6 PASS: Recent files limit and deduplication" << std::endl;
        ++passed;
    }

    // --- Test 7: Version info ---
    {
        WelcomeScreen ws;
        assert(!ws.getVersion().empty() && "Should have a version");
        ws.setVersion("1.0.0");
        assert(ws.getVersion() == "1.0.0" && "Version should be updated");

        std::cout << "Test 7 PASS: Version info accessible" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 55 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
