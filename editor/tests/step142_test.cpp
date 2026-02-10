// Step 142 TDD Test: Emacs package browser
#include "EmacsPackageBrowser.h"
#include "EmacsIntegration.h"
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

static const EmacsPackageEntry* findPkg(const std::vector<EmacsPackageEntry>& pkgs,
                                        const std::string& name) {
    for (const auto& pkg : pkgs) {
        if (pkg.name == name) return &pkg;
    }
    return nullptr;
}

int main() {
    int passed = 0;
    int failed = 0;

    MockEmacsConnection mock;
    EmacsPackageBrowserState state;
    std::string log;

    refreshEmacsPackages(state, mock, log);
    expect(!state.packages.empty(), "packages loaded", passed, failed);

    const EmacsPackageEntry* usePkg = findPkg(state.packages, "use-package");
    expect(usePkg != nullptr, "loaded package listed", passed, failed);
    if (usePkg) {
        expect(usePkg->status == "loaded", "loaded status set", passed, failed);
    }

    const EmacsPackageEntry* magitPkg = findPkg(state.packages, "magit");
    expect(magitPkg != nullptr, "available package listed", passed, failed);
    if (magitPkg) {
        expect(magitPkg->status == "available", "available status set", passed, failed);
    }

    bool loadOk = loadEmacsPackage(mock, "magit", log);
    expect(loadOk, "load package command", passed, failed);
    expect(mock.getLastSentCommand().find("(require 'magit)") != std::string::npos,
           "require command sent", passed, failed);

    std::cout << "\n=== Step 142 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
