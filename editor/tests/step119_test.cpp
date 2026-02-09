// Step 119 TDD Test: Zoom utilities
#include "ZoomUtils.h"
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

    expect(clampFontSize(10) == 12, "clamp low", passed, failed);
    expect(clampFontSize(30) == 24, "clamp high", passed, failed);
    expect(clampFontSize(18) == 18, "clamp ok", passed, failed);
    expect(zoomPercent(15, 15.0f) == 100, "zoom 100", passed, failed);
    expect(zoomPercent(18, 15.0f) == 120, "zoom 120", passed, failed);

    std::cout << "\n=== Step 119 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
