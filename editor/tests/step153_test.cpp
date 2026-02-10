// Step 153 TDD Test: Language coverage integration
#include "ast/Parser.h"
#include "ast/Generator.h"
#include "CrossLanguageProjector.h"
#include <iostream>
#include <vector>
#include <unordered_map>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

static std::unique_ptr<Module> parseForLanguage(const std::string& source,
                                                const std::string& language) {
    if (language == "python") return TreeSitterParser::parsePython(source);
    if (language == "cpp") return TreeSitterParser::parseCpp(source);
    if (language == "elisp") return TreeSitterParser::parseElisp(source);
    if (language == "javascript") return TreeSitterParser::parseJavaScript(source);
    if (language == "typescript") return TreeSitterParser::parseTypeScript(source);
    if (language == "java") return TreeSitterParser::parseJava(source);
    if (language == "rust") return TreeSitterParser::parseRust(source);
    if (language == "go") return TreeSitterParser::parseGo(source);
    return nullptr;
}

static std::string generateForLanguage(const ASTNode* ast,
                                       const std::string& language) {
    if (!ast) return "";
    if (language == "python") { PythonGenerator gen; return gen.generate(ast); }
    if (language == "cpp") { CppGenerator gen; return gen.generate(ast); }
    if (language == "elisp") { ElispGenerator gen; return gen.generate(ast); }
    if (language == "javascript") { JavaScriptGenerator gen; return gen.generate(ast); }
    if (language == "typescript") { TypeScriptGenerator gen; return gen.generate(ast); }
    if (language == "java") { JavaGenerator gen; return gen.generate(ast); }
    if (language == "rust") { RustGenerator gen; return gen.generate(ast); }
    if (language == "go") { GoGenerator gen; return gen.generate(ast); }
    return "";
}

int main() {
    int passed = 0;
    int failed = 0;

    std::unordered_map<std::string, std::string> samples = {
        {"python", R"(def add(a, b):
    return a + b
)"},
        {"cpp", R"(int add(int a, int b) {
    return a + b;
}
)"},
        {"elisp", R"((defun add (a b) (+ a b)))"},
        {"javascript", R"(function add(a, b) { return a + b; })"},
        {"typescript", R"(function add(a: number, b: number): number { return a + b; })"},
        {"java", R"(public class Box { public int size(int n) { return n + 1; } })"},
        {"rust", R"(fn add(a: i32, b: i32) -> i32 { return a + b; })"},
        {"go", R"(package main
func Add(a int, b int) int { return a + b })"}
    };

    std::vector<std::string> languages = {
        "python", "cpp", "elisp", "javascript",
        "typescript", "java", "rust", "go"
    };

    std::unordered_map<std::string, std::unique_ptr<Module>> parsed;
    for (const auto& lang : languages) {
        auto it = samples.find(lang);
        if (it == samples.end()) continue;
        auto mod = parseForLanguage(it->second, lang);
        expect(mod != nullptr, lang + " parsed", passed, failed);
        parsed[lang] = std::move(mod);
    }

    for (const auto& lang : languages) {
        auto it = parsed.find(lang);
        if (it == parsed.end()) continue;
        std::string generated = generateForLanguage(it->second.get(), lang);
        expect(!generated.empty(), lang + " generated", passed, failed);
    }

    CrossLanguageProjector projector;
    for (const auto& srcLang : languages) {
        auto it = parsed.find(srcLang);
        if (it == parsed.end()) continue;
        for (const auto& dstLang : languages) {
            auto projected = projector.project(it->second.get(), dstLang);
            expect(projected != nullptr, srcLang + "->" + dstLang + " projected", passed, failed);
            expect(projected && projected->targetLanguage == dstLang,
                   srcLang + "->" + dstLang + " target set", passed, failed);
            expect(projector.annotationsPreserved(it->second.get(), projected.get()),
                   srcLang + "->" + dstLang + " annotations preserved", passed, failed);
        }
    }

    std::cout << "\n=== Step 153 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
