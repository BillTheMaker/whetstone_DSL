// Step 87 TDD Test: Welcome screen
//
// Tests:
// 1. Default actions and tips are present
// 2. Recent files added and de-duplicated

#include <cassert>
#include <iostream>
#include "WelcomeScreen.h"

int main() {
    int passed = 0;
    int failed = 0;

    WelcomeScreen w;
    assert(!w.getActions().empty());
    assert(!w.getTips().empty());
    std::cout << "Test 1 PASS: Actions and tips" << std::endl;
    ++passed;

    w.addRecentFile("C:/tmp/a.py", "python");
    w.addRecentFile("C:/tmp/a.py", "python");
    assert(w.getRecentFiles().size() == 1);
    std::cout << "Test 2 PASS: Recent files de-duplicate" << std::endl;
    ++passed;

    std::cout << "\n=== Step 87 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
