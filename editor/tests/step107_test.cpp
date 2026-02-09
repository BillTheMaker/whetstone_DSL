// Step 107 TDD Test: Diff view utilities
//
// Tests:
// 1. buildLineDiff marks changed lines on both sides

#include <cassert>
#include <iostream>
#include "DiffUtils.h"

int main() {
    int passed = 0;
    int failed = 0;

    std::string before = "a\nb\nc";
    std::string after = "a\nx\nc\nd";
    auto diff = buildLineDiff(before, after);
    assert(diff.beforeLines.size() == 1);
    assert(diff.beforeLines[0] == 1);
    assert(diff.afterLines.size() == 2);
    assert(diff.afterLines[0] == 1);
    assert(diff.afterLines[1] == 3);
    std::cout << "Test 1 PASS: diff lines detected" << std::endl;
    ++passed;

    std::cout << "\n=== Step 107 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
