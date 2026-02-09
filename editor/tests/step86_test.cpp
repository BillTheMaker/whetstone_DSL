// Step 86 TDD Test: Multi-tab editing (BufferManager)
//
// Tests:
// 1. Opening buffers sets active
// 2. Switching active buffer works
// 3. Closing active buffer selects another

#include <cassert>
#include <iostream>
#include "BufferManager.h"

int main() {
    int passed = 0;
    int failed = 0;

    BufferManager bm;
    bm.openBuffer("a.py", "print(1)", "python");
    assert(bm.getActiveBufferPath() == "a.py");
    bm.openBuffer("b.py", "print(2)", "python");
    assert(bm.getActiveBufferPath() == "b.py");
    std::cout << "Test 1 PASS: Opening buffers sets active" << std::endl;
    ++passed;

    bm.switchToBuffer("a.py");
    assert(bm.getActiveBufferPath() == "a.py");
    std::cout << "Test 2 PASS: Switching active buffer" << std::endl;
    ++passed;

    bm.closeBuffer("a.py");
    assert(bm.getActiveBufferPath() == "b.py");
    std::cout << "Test 3 PASS: Closing active selects another" << std::endl;
    ++passed;

    std::cout << "\n=== Step 86 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
