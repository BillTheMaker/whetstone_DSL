// Step 88 TDD Test: Drag-and-drop handler
//
// Tests:
// 1. File drop invokes file callback
// 2. Folder drop invokes folder callback

#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "DragDropHandler.h"

int main() {
    int passed = 0;
    int failed = 0;

    std::filesystem::path root = std::filesystem::current_path() / "tmp_step88";
    std::filesystem::create_directories(root / "dir");
    std::filesystem::path file = root / "file.txt";
    std::ofstream f(file.string());
    f << "x";
    f.close();

    bool fileCalled = false;
    bool folderCalled = false;

    DragDropHandler::handleDrop(file.string(),
        [&](const std::string& p){ fileCalled = (p == file.string()); },
        [&](const std::string& p){ folderCalled = (p == (root / "dir").string()); }
    );
    assert(fileCalled);
    std::cout << "Test 1 PASS: File drop" << std::endl;
    ++passed;

    fileCalled = false;
    folderCalled = false;
    DragDropHandler::handleDrop((root / "dir").string(),
        [&](const std::string& p){ fileCalled = (p == file.string()); },
        [&](const std::string& p){ folderCalled = (p == (root / "dir").string()); }
    );
    assert(folderCalled);
    std::cout << "Test 2 PASS: Folder drop" << std::endl;
    ++passed;

    std::filesystem::remove_all(root);

    std::cout << "\n=== Step 88 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
