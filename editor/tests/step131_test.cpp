// Step 131 TDD Test: Library API indexing via type stubs
#include "StubParser.h"
#include "LibraryIndexer.h"
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

    std::filesystem::path dir = std::filesystem::current_path() / "stub_test_tmp";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir / "mylib");

    {
        std::ofstream out(dir / "mylib" / "mylib.pyi");
        out << "def foo(x: int) -> int: ...\nclass Bar: ...\nCONST: int\n";
    }
    {
        std::ofstream out(dir / "mylib" / "index.d.ts");
        out << "export function baz(x: number): number;\nexport interface Thing {}\n";
    }
    {
        std::ofstream out(dir / "mylib" / "mylib.h");
        out << "int add(int a, int b);\nclass Widget;\n";
    }
    {
        std::filesystem::create_directories(dir / "mylib" / "src");
        std::ofstream out(dir / "mylib" / "src" / "lib.rs");
        out << "pub fn qux(x: i32) -> i32 { x }\npub struct Gizmo {}\n";
    }

    auto py = StubParser::parseFile((dir / "mylib" / "mylib.pyi").string(), "python");
    expect(py.size() >= 2, "python stub parsed", passed, failed);

    auto ts = StubParser::parseFile((dir / "mylib" / "index.d.ts").string(), "typescript");
    expect(ts.size() >= 2, "typescript stub parsed", passed, failed);

    auto hdr = StubParser::parseFile((dir / "mylib" / "mylib.h").string(), "cpp");
    expect(!hdr.empty(), "header stub parsed", passed, failed);

    auto rs = StubParser::parseFile((dir / "mylib" / "src" / "lib.rs").string(), "rust");
    expect(rs.size() >= 2, "rust stub parsed", passed, failed);

    auto scanned = StubParser::scanWorkspaceForLibrary(dir.string(), "mylib", "python");
    expect(!scanned.empty(), "scan workspace finds stubs", passed, failed);

    LibraryIndexData index;
    std::vector<DependencySpec> deps;
    deps.push_back({"mylib", "0.1.0", "requirements.txt"});
    applyStubFallback(index, deps, dir.string());
    expect(index.completionsByLibrary.count("mylib") == 1, "stub fallback fills index", passed, failed);

    Module mod("m1", "Module", "python");
    rebuildExternalModules(&mod, deps, index);
    auto ext = mod.getChildren("externalModules");
    expect(!ext.empty(), "external module created from stubs", passed, failed);
    if (!ext.empty()) {
        auto sigs = ext[0]->getChildren("signatures");
        expect(!sigs.empty(), "signatures attached", passed, failed);
    }

    std::filesystem::remove_all(dir);

    std::cout << "\n=== Step 131 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
