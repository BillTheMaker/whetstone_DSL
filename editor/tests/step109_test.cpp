// Step 109 TDD Test: Command palette
//
// Tests:
// 1. Fuzzy search matches and orders
// 2. MRU ordering when query empty

#include <cassert>
#include <iostream>
#include "CommandPalette.h"

int main() {
    int passed = 0;
    int failed = 0;

    CommandPalette palette;
    palette.registerCommand("file.open", "File: Open", "Ctrl+O");
    palette.registerCommand("file.save", "File: Save", "Ctrl+S");
    palette.registerCommand("edit.find", "Edit: Find", "Ctrl+F");

    auto results = palette.search("op");
    assert(!results.empty());
    assert(results[0].id == "file.open");
    std::cout << "Test 1 PASS: fuzzy search" << std::endl;
    ++passed;

    palette.markUsed("edit.find");
    palette.markUsed("file.save");
    auto all = palette.search("");
    assert(all.size() == 3);
    assert(all[0].id == "file.save");
    std::cout << "Test 2 PASS: MRU ordering" << std::endl;
    ++passed;

    std::cout << "\n=== Step 109 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
