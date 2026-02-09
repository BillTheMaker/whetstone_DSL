// Step 85 TDD Test: Filesystem tree and gitignore
//
// Tests:
// 1. Builds tree with files and folders
// 2. Respects .gitignore patterns

#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "FileTree.h"

static void writeFile(const std::filesystem::path& p, const std::string& text) {
    std::ofstream f(p.string(), std::ios::binary);
    f << text;
}

int main() {
    int passed = 0;
    int failed = 0;

    std::filesystem::path root = std::filesystem::current_path() / "tmp_step85";
    std::filesystem::create_directories(root / "src");
    std::filesystem::create_directories(root / "ignoredir");

    writeFile(root / "src" / "main.cpp", "int main(){}\n");
    writeFile(root / "ignore.tmp", "x");
    writeFile(root / "ignoredir" / "foo.txt", "y");
    writeFile(root / ".gitignore", "ignoredir/\n*.tmp\n");

    FileTree tree;
    auto node = tree.build(root.string());
    assert(node.isDir);

    bool sawMain = false;
    bool sawIgnored = false;
    for (const auto& child : node.children) {
        if (child.name == "src") {
            for (const auto& c2 : child.children) {
                if (c2.name == "main.cpp") sawMain = true;
            }
        }
        if (child.name == "ignoredir" || child.name == "ignore.tmp") {
            sawIgnored = true;
        }
    }

    assert(sawMain);
    assert(!sawIgnored);

    std::cout << "Test 1 PASS: Tree build + gitignore" << std::endl;
    ++passed;

    std::filesystem::remove_all(root);

    std::cout << "\n=== Step 85 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
