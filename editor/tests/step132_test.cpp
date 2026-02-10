// Step 132 TDD Test: Library symbol browser panel helpers
#include "LibraryBrowserPanel.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    expect(formatUsageTemplate("foo") == "foo()", "usage template adds parens", passed, failed);
    expect(formatUsageTemplate("Widget") == "Widget", "usage template keeps class", passed, failed);
    expect(symbolMatchesFilter("numpy.array", "ARRAY"), "filter is case-insensitive", passed, failed);

    LibraryIndexData index;
    index.symbolsByLibrary["numpy"].push_back({"numpy.array", "numpy", "Create array", 0});
    LibraryBrowserState state;
    updateDocText(state, index, "numpy", "numpy.array");
    expect(state.docText == "Create array", "doc text from LSP detail", passed, failed);

    LibraryBrowserState fallbackState;
    updateDocText(fallbackState, index, "missing", "missing");
    expect(fallbackState.docText == "No documentation available.", "fallback doc text", passed, failed);

    std::cout << "\n=== Step 132 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
