#pragma once

// Step 483: Convention Extractor + Validator
// Extracts machine-readable project conventions and validates step outputs.

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

struct ProjectConventions {
    std::string testHarnessPattern = "TEST/CHECK macros";
    std::string assertionStyle = "CHECK/assert";
    std::map<std::string, int> sizeLimits; // header, main.cpp, function
    std::string buildSystem = "cmake";
    std::string buildTargetPattern = "add_executable(stepNNN_test tests/stepNNN_test.cpp)";
    bool headerOnly = true;
    std::string classNaming = "PascalCase";
    std::string methodNaming = "camelCase";
    std::string fileNaming = "PascalCase.h";
};

struct ConventionViolation {
    std::string code;
    std::string filePath;
    std::string message;
    std::string severity = "warning";
};

struct ConventionValidationReport {
    std::vector<ConventionViolation> violations;

    bool hasViolation(const std::string& code) const {
        for (const auto& v : violations) {
            if (v.code == code) return true;
        }
        return false;
    }
};

struct SourceFileDraft {
    std::string path;
    std::string content;
};

class ProjectConventionAnalyzer {
public:
    static ProjectConventions extract(const std::string& repoRoot) {
        ProjectConventions out;

        const std::string architectureText = readAll(findFirstExisting(repoRoot, {
            "ARCHITECTURE.md", "architecture.md"
        }));
        if (!architectureText.empty()) {
            maybeSetLimit(architectureText, "Header file", "header", out.sizeLimits);
            maybeSetLimit(architectureText, "main.cpp", "main.cpp", out.sizeLimits);
            maybeSetLimit(architectureText, "Single function", "function", out.sizeLimits);
            if (contains(architectureText, "Header-Only")) out.headerOnly = true;
            if (contains(architectureText, "PascalCase")) out.classNaming = "PascalCase";
            if (contains(architectureText, "camelCase")) out.methodNaming = "camelCase";
        }

        const std::string cmakeText = readAll(repoRoot + "/editor/CMakeLists.txt");
        if (contains(cmakeText, "add_executable(step")) {
            out.buildSystem = "cmake";
            out.buildTargetPattern = "add_executable(stepNNN_test tests/stepNNN_test.cpp)";
        }

        const std::string sampleTest = readFirstMatching(repoRoot + "/editor/tests", "step*_test.cpp");
        if (contains(sampleTest, "#define TEST(") && contains(sampleTest, "#define CHECK(")) {
            out.testHarnessPattern = "TEST/PASS/FAIL/CHECK macro harness";
        }
        if (contains(sampleTest, "assert(")) {
            out.assertionStyle = "CHECK/assert";
        }

        if (!out.sizeLimits.count("header")) out.sizeLimits["header"] = 600;
        if (!out.sizeLimits.count("main.cpp")) out.sizeLimits["main.cpp"] = 1500;
        if (!out.sizeLimits.count("function")) out.sizeLimits["function"] = 80;
        return out;
    }

    static ConventionValidationReport validate(const ProjectConventions& c,
                                               const std::vector<SourceFileDraft>& files) {
        ConventionValidationReport out;
        for (const auto& f : files) {
            const int lineCount = countLines(f.content);
            if (isHeader(f.path) && lineCount > getOrDefault(c.sizeLimits, "header", 600)) {
                out.violations.push_back({"HEADER_TOO_LARGE", f.path,
                    "Header exceeds size limit", "warning"});
            }
            if (isMainCpp(f.path) && lineCount > getOrDefault(c.sizeLimits, "main.cpp", 1500)) {
                out.violations.push_back({"MAIN_TOO_LARGE", f.path,
                    "main.cpp exceeds size limit", "warning"});
            }
            if (isTestFile(f.path)) {
                const bool hasCheck = contains(f.content, "CHECK(") || contains(f.content, "assert(");
                if (!hasCheck) {
                    out.violations.push_back({"TEST_ASSERTION_MISSING", f.path,
                        "Test file missing CHECK/assert assertions", "warning"});
                }
            }
            if (isHeader(f.path)) {
                validateClassNaming(f, out);
            }
            if (f.path.find("CMakeLists.txt") != std::string::npos &&
                f.content.find("add_executable(step") == std::string::npos) {
                out.violations.push_back({"BUILD_TARGET_MISSING", f.path,
                    "No step test target found in CMake snippet", "warning"});
            }
        }
        return out;
    }

private:
    static bool contains(const std::string& text, const std::string& needle) {
        return text.find(needle) != std::string::npos;
    }

    static int countLines(const std::string& text) {
        if (text.empty()) return 0;
        int lines = 1;
        for (char c : text) if (c == '\n') ++lines;
        return lines;
    }

    static int getOrDefault(const std::map<std::string, int>& m,
                            const std::string& key, int fallback) {
        auto it = m.find(key);
        if (it == m.end()) return fallback;
        return it->second;
    }

    static bool isHeader(const std::string& path) {
        return endsWith(path, ".h") || endsWith(path, ".hpp");
    }

    static bool isMainCpp(const std::string& path) {
        return endsWith(path, "main.cpp");
    }

    static bool isTestFile(const std::string& path) {
        return path.find("/tests/") != std::string::npos && endsWith(path, "_test.cpp");
    }

    static bool endsWith(const std::string& value, const std::string& suffix) {
        return value.size() >= suffix.size() &&
               value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    static std::string readAll(const std::string& path) {
        if (path.empty()) return "";
        std::ifstream in(path);
        if (!in.is_open()) return "";
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    static std::string findFirstExisting(const std::string& root,
                                         std::initializer_list<const char*> candidates) {
        for (const auto& c : candidates) {
            const std::string p = root + "/" + c;
            if (std::filesystem::exists(p)) return p;
        }
        return "";
    }

    static std::string readFirstMatching(const std::string& dir, const std::string& patternGlob) {
        (void)patternGlob; // Simplified: read first step test file found.
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec)) return "";
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (ec) break;
            if (!entry.is_regular_file()) continue;
            auto name = entry.path().filename().string();
            if (name.find("step") == 0 && name.find("_test.cpp") != std::string::npos) {
                return readAll(entry.path().string());
            }
        }
        return "";
    }

    static void maybeSetLimit(const std::string& architectureText,
                              const std::string& key,
                              const std::string& outKey,
                              std::map<std::string, int>& limits) {
        std::regex row("\\|\\s*" + key + "\\s*\\|\\s*([0-9]+)\\s*\\|");
        std::smatch m;
        if (std::regex_search(architectureText, m, row) && m.size() > 1) {
            limits[outKey] = std::stoi(m[1].str());
        }
    }

    static bool isPascalCase(const std::string& name) {
        if (name.empty()) return false;
        if (!std::isupper(static_cast<unsigned char>(name[0]))) return false;
        for (char c : name) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (!std::isalnum(uc) && c != '_') return false;
        }
        return true;
    }

    static void validateClassNaming(const SourceFileDraft& f,
                                    ConventionValidationReport& out) {
        std::regex classRe("\\b(class|struct)\\s+([A-Za-z_][A-Za-z0-9_]*)");
        auto begin = std::sregex_iterator(f.content.begin(), f.content.end(), classRe);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            std::string cls = (*it)[2].str();
            if (!isPascalCase(cls)) {
                out.violations.push_back({"CLASS_NAMING_MISMATCH", f.path,
                    "Class/struct should use PascalCase: " + cls, "warning"});
            }
        }
    }
};
