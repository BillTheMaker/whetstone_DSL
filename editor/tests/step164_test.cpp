// Step 164 TDD Test: Update checker (file manifest)
#include "UpdateChecker.h"
#include <iostream>
#include <filesystem>
#include <fstream>

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

    namespace fs = std::filesystem;
    fs::path root = fs::temp_directory_path() / "whetstone_update_test";
    fs::create_directories(root);
    fs::path manifest = root / "manifest.json";

    std::ofstream out(manifest.string());
    out << "{ \"version\": \"1.2.3\", \"url\": \"https://example.com\", \"notes\": \"test\" }";
    out.close();

    UpdateChecker checker;
    UpdateInfo info = checker.check(std::string("file://") + manifest.string());
    expect(info.available, "update available", passed, failed);
    expect(info.version == "1.2.3", "version parsed", passed, failed);
    expect(info.url == "https://example.com", "url parsed", passed, failed);

    std::cout << "\n=== Step 164 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
