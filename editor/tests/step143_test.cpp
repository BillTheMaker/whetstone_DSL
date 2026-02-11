// Step 143 TDD Test: Elisp function discovery and indexing
#include "EmacsFunctionDiscovery.h"
#include "NotificationSystem.h"
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

    MockEmacsConnection mock;
    NotificationSystem notifications;
    std::string error;

    auto packages = queryEmacsPackageList(mock, false, notifications, error);
    expect(!packages.empty(), "loaded packages fetched", passed, failed);

    auto funcs = queryEmacsFunctions(mock, "^use-package", notifications);
    expect(!funcs.empty(), "apropos returns functions", passed, failed);

    auto doc = queryEmacsFunctionDoc(mock, "use-package", notifications);
    expect(!doc.signature.empty(), "describe-function signature", passed, failed);
    expect(!doc.doc.empty(), "describe-function docstring", passed, failed);

    EmacsFunctionIndex index;
    refreshEmacsFunctionIndex(index, mock, packages, notifications);
    expect(index.functionsByPackage.count("use-package") > 0, "index contains use-package", passed, failed);

    Module module("mod1", "test", "elisp");
    int sigId = 0;
    appendEmacsExternalModules(&module, index, sigId);
    auto extMods = module.getChildren("externalModules");
    expect(!extMods.empty(), "external modules appended", passed, failed);

    LibraryIndexData libIndex;
    addEmacsSymbolsToLibraryIndex(libIndex, index);
    expect(libIndex.symbolsByLibrary.count("use-package") > 0, "library index updated", passed, failed);

    std::cout << "\n=== Step 143 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
