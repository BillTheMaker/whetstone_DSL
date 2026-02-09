// Step 107c TDD Test: Recent file mode persistence
//
// Tests:
// 1. Recent files store mode string
// 2. Re-adding updates mode

#include <cassert>
#include <iostream>
#include "WelcomeScreen.h"

int main() {
    int passed = 0;
    int failed = 0;

    WelcomeScreen welcome;
    welcome.addRecentFile("/tmp/a.py", "python", "text");
    auto recents = welcome.getRecentFiles();
    assert(recents.size() == 1);
    assert(recents[0].mode == "text");
    std::cout << "Test 1 PASS: mode stored" << std::endl;
    ++passed;

    welcome.addRecentFile("/tmp/a.py", "python", "structured");
    recents = welcome.getRecentFiles();
    assert(recents.size() == 1);
    assert(recents[0].mode == "structured");
    std::cout << "Test 2 PASS: mode updated" << std::endl;
    ++passed;

    std::cout << "\n=== Step 107c Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
