// Step 139 TDD Test: Library compatibility matrix
#include "LibraryCompatibility.h"
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

    LibraryCompatibility compat;
    compat.loadDefaultMappings();
    auto mappings = compat.getMappings("numpy", "python");
    expect(!mappings.empty(), "numpy mappings present", passed, failed);
    bool hasEigen = false;
    for (const auto& entry : mappings) {
        if (entry.first == "Eigen" && entry.second == "cpp") {
            hasEigen = true;
        }
    }
    expect(hasEigen, "numpy -> Eigen mapping", passed, failed);

    compat.addMapping("numpy", "python", "armadillo", "cpp");
    auto mappings2 = compat.getMappings("numpy", "python");
    bool hasArma = false;
    for (const auto& entry : mappings2) {
        if (entry.first == "armadillo" && entry.second == "cpp") {
            hasArma = true;
        }
    }
    expect(hasArma, "custom mapping added", passed, failed);

    std::cout << "\n=== Step 139 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
