// Step 113 TDD Test: Project search
#include "ProjectSearch.h"
#include <iostream>
#include <filesystem>
#include <fstream>

static void writeFile(const std::filesystem::path& p, const std::string& content) {
    std::filesystem::create_directories(p.parent_path());
    std::ofstream out(p.string(), std::ios::binary);
    out << content;
}

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

    std::filesystem::path root = std::filesystem::temp_directory_path() / "whetstone_search_test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    writeFile(root / "a.txt", "hello world\nsecond line\n");
    writeFile(root / "b.log", "hello log\n");
    writeFile(root / "sub" / "c.txt", "no match here\nhello again\n");

    ProjectSearch search;
    search.setRoot(root.string());

    {
        auto results = search.search("hello", "", "", false);
        bool ok = results.size() == 3;
        expect(ok, "basic search", passed, failed);
    }

    {
        auto results = search.search("hello", "*.txt", "", false);
        bool ok = results.size() == 2;
        expect(ok, "include glob", passed, failed);
    }

    {
        auto results = search.search("hello", "", "b.log", false);
        bool ok = results.size() == 2;
        expect(ok, "exclude glob", passed, failed);
    }

    std::filesystem::remove_all(root);

    std::cout << "\n=== Step 113 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
