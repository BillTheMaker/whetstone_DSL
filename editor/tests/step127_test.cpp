// Step 127 TDD Test: Package registry abstraction
#include "PackageRegistry.h"
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

    PackageInfo py = PackageRegistry::query(PackageEcosystem::Python, "numpy");
    expect(py.name == "numpy", "python name", passed, failed);
    expect(!py.homepageUrl.empty(), "python url", passed, failed);

    PackageInfo npm = PackageRegistry::query(PackageEcosystem::Npm, "react");
    expect(npm.name == "react", "npm name", passed, failed);
    expect(!npm.description.empty(), "npm description", passed, failed);

    std::cout << "\n=== Step 127 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
