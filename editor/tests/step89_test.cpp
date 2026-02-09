// Step 89 TDD Test: FileWatcher
//
// Tests:
// 1. Detects change on file modification

#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include "FileWatcher.h"

int main() {
    int passed = 0;
    int failed = 0;

    std::filesystem::path root = std::filesystem::current_path() / "tmp_step89";
    std::filesystem::create_directories(root);
    std::filesystem::path file = root / "file.txt";
    {
        std::ofstream f(file.string());
        f << "a";
    }

    FileWatcher watcher;
    watcher.watch(file.string());

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    {
        std::ofstream f(file.string());
        f << "b";
    }

    auto changed = watcher.poll();
    assert(!changed.empty());
    std::cout << "Test 1 PASS: Change detected" << std::endl;
    ++passed;

    std::filesystem::remove_all(root);

    std::cout << "\n=== Step 89 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
