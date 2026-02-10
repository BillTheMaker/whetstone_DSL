// Step 133 TDD Test: Import statement generation + unused import detection
#include "ImportManager.h"
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

    std::string py = "import os\n\nprint('hi')\n";
    auto pyRes = ensureImport(py, "python", "numpy", "array");
    expect(pyRes.changed, "python import changed", passed, failed);
    expect(pyRes.text.find("from numpy import array") != std::string::npos,
           "python import inserted", passed, failed);

    std::string js = "import { useEffect } from 'react';\n\nconsole.log('x');\n";
    auto jsRes = ensureImport(js, "javascript", "react", "useState");
    expect(jsRes.changed, "js import changed", passed, failed);
    expect(jsRes.text.find("import { useState } from 'react';") != std::string::npos,
           "js import inserted", passed, failed);

    std::string go = "package main\n\nfunc main() {}\n";
    auto goRes = ensureImport(go, "go", "fmt", "fmt");
    expect(goRes.changed, "go import changed", passed, failed);
    expect(goRes.text.find("import (\n    \"fmt\"\n)") != std::string::npos,
           "go import block formatting", passed, failed);

    std::string pyUnused = "import os\nprint('hi')\n";
    auto issues = findUnusedImports(pyUnused, "python");
    expect(!issues.empty(), "unused import detected", passed, failed);

    std::cout << "\n=== Step 133 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
