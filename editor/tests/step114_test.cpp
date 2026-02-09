// Step 114 TDD Test: Go-to-line parsing
#include "GoToLine.h"
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

    {
        int line = 0, col = 0;
        bool ok = parseLineColInput("10", line, col) && line == 10 && col == 1;
        expect(ok, "line only", passed, failed);
    }

    {
        int line = 0, col = 0;
        bool ok = parseLineColInput(":7:3", line, col) && line == 7 && col == 3;
        expect(ok, "colon prefix", passed, failed);
    }

    {
        int line = 0, col = 0;
        bool ok = parseLineColInput("5:9", line, col) && line == 5 && col == 9;
        expect(ok, "line and col", passed, failed);
    }

    {
        int line = 0, col = 0;
        bool ok = !parseLineColInput("abc", line, col);
        expect(ok, "invalid input", passed, failed);
    }

    std::cout << "\n=== Step 114 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
