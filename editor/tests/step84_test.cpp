// Step 84 TDD Test: Native file dialogs wrapper
//
// Tests:
// 1. Provider override returns expected open/save paths
// 2. Clearing provider falls back to empty (non-interactive)

#include <cassert>
#include <iostream>
#include "FileDialog.h"

static std::string g_open;
static std::string g_save;
static std::string g_folder;

static std::string openProvider(const FileDialog::OpenRequest& req) {
    (void)req;
    return g_open;
}

static std::string saveProvider(const FileDialog::SaveRequest& req) {
    (void)req;
    return g_save;
}

static std::string folderProvider(const FileDialog::FolderRequest& req) {
    (void)req;
    return g_folder;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Provider override ---
    {
        g_open = "C:/tmp/foo.py";
        g_save = "C:/tmp/out.py";
        g_folder = "C:/tmp/project";
        FileDialog::Provider provider{openProvider, saveProvider, folderProvider};
        FileDialog::setProvider(provider);

        auto open = FileDialog::openFile({"Open", {"*.py"}, ""});
        auto save = FileDialog::saveFile({"Save", {"*.py"}, ""});
        auto folder = FileDialog::openFolder({"Folder", ""});

        assert(open == g_open);
        assert(save == g_save);
        assert(folder == g_folder);
        std::cout << "Test 1 PASS: Provider override" << std::endl;
        ++passed;
    }

    // --- Test 2: Clear provider ---
    {
        FileDialog::clearProvider();
        auto open = FileDialog::openFile({"Open", {"*.py"}, ""});
        assert(open.empty());
        std::cout << "Test 2 PASS: Clear provider" << std::endl;
        ++passed;
    }

    std::cout << "\n=== Step 84 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
