// Step 107a TDD Test: Text editor mode toggle persistence in BufferManager
//
// Tests:
// 1. Default mode is Structured
// 2. setBufferMode updates mode

#include <cassert>
#include <iostream>
#include "BufferManager.h"

int main() {
    int passed = 0;
    int failed = 0;

    BufferManager bm;
    bm.openBuffer("file1", "", "python");
    assert(bm.getBufferMode("file1") == BufferManager::BufferMode::Structured);
    std::cout << "Test 1 PASS: default mode structured" << std::endl;
    ++passed;

    bm.setBufferMode("file1", BufferManager::BufferMode::Text);
    assert(bm.getBufferMode("file1") == BufferManager::BufferMode::Text);
    std::cout << "Test 2 PASS: mode updated" << std::endl;
    ++passed;

    std::cout << "\n=== Step 107a Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
