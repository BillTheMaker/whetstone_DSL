// Step 107b TDD Test: Editor mode policy
//
// Tests:
// 1. Structured mode allows structured features
// 2. Text mode disables structured features

#include <cassert>
#include <iostream>
#include "EditorModePolicy.h"

int main() {
    int passed = 0;
    int failed = 0;

    assert(allowStructuredFeatures(BufferManager::BufferMode::Structured));
    assert(!isTextMode(BufferManager::BufferMode::Structured));
    std::cout << "Test 1 PASS: structured mode allowed" << std::endl;
    ++passed;

    assert(!allowStructuredFeatures(BufferManager::BufferMode::Text));
    assert(isTextMode(BufferManager::BufferMode::Text));
    std::cout << "Test 2 PASS: text mode disabled" << std::endl;
    ++passed;

    std::cout << "\n=== Step 107b Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
