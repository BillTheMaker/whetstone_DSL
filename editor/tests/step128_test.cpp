// Step 128 TDD Test: Dependency parsing
#include "DependencyParser.h"
#include <iostream>
#include <filesystem>

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

    std::filesystem::path dir = std::filesystem::current_path() / "dep_test_tmp";
    std::filesystem::create_directories(dir);

    // requirements.txt
    {
        std::ofstream out(dir / "requirements.txt");
        out << "numpy==1.26.0\n# comment\nrequests>=2.0\n";
    }
    auto reqs = DependencyParser::parseFile((dir / "requirements.txt").string());
    expect(reqs.size() == 2, "requirements count", passed, failed);
    if (reqs.size() >= 2) {
        expect(reqs[0].name == "numpy", "requirements name", passed, failed);
        expect(reqs[0].version == "1.26.0", "requirements version", passed, failed);
    }

    // package.json
    {
        std::ofstream out(dir / "package.json");
        out << "{ \"dependencies\": { \"react\": \"^18.2.0\" },"
               "\"devDependencies\": { \"vite\": \"^5.0.0\" } }";
    }
    auto npm = DependencyParser::parseFile((dir / "package.json").string());
    expect(npm.size() == 2, "package.json count", passed, failed);

    // Cargo.toml
    {
        std::ofstream out(dir / "Cargo.toml");
        out << "[dependencies]\nserde = \"1.0\"\nanyhow = { version = \"1.0\" }\n";
    }
    auto cargo = DependencyParser::parseFile((dir / "Cargo.toml").string());
    expect(cargo.size() == 2, "cargo count", passed, failed);

    // Populate imports
    Module mod("m1", "Mod", "python");
    DependencyParser::populateImports(&mod, reqs);
    expect(mod.getChildren("imports").size() == 2, "imports populated", passed, failed);

    std::filesystem::remove_all(dir);

    std::cout << "\n=== Step 128 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
