#pragma once

// Step 493: Parse Full Whetstone Codebase
// Audits C++ parser coverage across header files.

#include "SelfHostHarness.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <string>
#include <vector>

struct FileParseAudit {
    std::string path;
    bool parseSuccess = false;
    int classCount = 0;
    int functionCount = 0;
    int annotationNodeCount = 0;
    int textualClassHints = 0;
    int textualFunctionHints = 0;
    std::vector<std::string> skippedConstructs;
    std::vector<std::string> warnings;
};

struct CodebaseParseAudit {
    std::vector<FileParseAudit> files;
    int totalFiles = 0;
    int parsedFiles = 0;
    int totalClasses = 0;
    int totalFunctions = 0;
    int totalAnnotationNodes = 0;
    int skippedClassConstructs = 0;
    int skippedFunctionConstructs = 0;
    std::vector<std::string> unparseableConstructLog;

    double parseRate() const {
        return totalFiles > 0 ? static_cast<double>(parsedFiles) / totalFiles : 1.0;
    }

    bool meetsTarget(double target = 0.95) const {
        return parseRate() >= target;
    }
};

class SelfHostCodebaseAudit {
public:
    static std::vector<std::string> listHeaderFiles(const std::vector<std::string>& roots) {
        std::set<std::string> unique;
        std::error_code ec;
        for (const auto& root : roots) {
            if (!std::filesystem::exists(root, ec)) continue;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(root, ec)) {
                if (ec) break;
                if (!entry.is_regular_file()) continue;
                const auto path = entry.path().string();
                if (endsWith(path, ".h")) unique.insert(path);
            }
        }
        return {unique.begin(), unique.end()};
    }

    static CodebaseParseAudit auditDirectories(const std::vector<std::string>& roots) {
        return auditFiles(listHeaderFiles(roots));
    }

    static CodebaseParseAudit auditFiles(const std::vector<std::string>& files) {
        CodebaseParseAudit out;
        out.totalFiles = static_cast<int>(files.size());
        for (const auto& path : files) {
            auto fa = auditFile(path);
            out.files.push_back(fa);
            if (fa.parseSuccess) out.parsedFiles++;
            out.totalClasses += fa.classCount;
            out.totalFunctions += fa.functionCount;
            out.totalAnnotationNodes += fa.annotationNodeCount;

            for (const auto& s : fa.skippedConstructs) {
                if (s == "class") out.skippedClassConstructs++;
                if (s == "function") out.skippedFunctionConstructs++;
                out.unparseableConstructLog.push_back(path + ": skipped " + s);
            }
            for (const auto& w : fa.warnings) {
                out.unparseableConstructLog.push_back(path + ": " + w);
            }
        }
        return out;
    }

private:
    static FileParseAudit auditFile(const std::string& path) {
        FileParseAudit out;
        out.path = path;

        const std::string source = SelfHostHarness::readFile(path);
        out.textualClassHints = countRegex(source, std::regex(R"(\b(class|struct)\s+[A-Za-z_])"));
        out.textualFunctionHints = countRegex(source, std::regex(R"(\b[A-Za-z_~][A-Za-z0-9_:<>~]*\s+[A-Za-z_~][A-Za-z0-9_]*\s*\([^;\)]*\)\s*[\{;])"));

        auto parsed = SelfHostHarness::parseFile(path);
        out.parseSuccess = parsed.parseSuccess;
        out.warnings = parsed.warnings;
        out.classCount = SelfHostHarness::countClasses(parsed.ast.get());
        out.functionCount = SelfHostHarness::countFunctions(parsed.ast.get());
        out.annotationNodeCount = countAnnotations(parsed.ast.get());

        if (out.textualClassHints > out.classCount) {
            out.skippedConstructs.push_back("class");
        }
        if (out.textualFunctionHints > out.functionCount) {
            out.skippedConstructs.push_back("function");
        }
        return out;
    }

    static int countRegex(const std::string& text, const std::regex& re) {
        if (text.empty()) return 0;
        auto begin = std::sregex_iterator(text.begin(), text.end(), re);
        auto end = std::sregex_iterator();
        int count = 0;
        for (auto it = begin; it != end; ++it) ++count;
        return count;
    }

    static bool endsWith(const std::string& value, const std::string& suffix) {
        return value.size() >= suffix.size() &&
               value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    static int countAnnotations(const ASTNode* node) {
        if (!node) return 0;
        int count = 0;
        if (node->conceptType.find("Annotation") != std::string::npos) ++count;
        for (auto* child : node->allChildren()) {
            count += countAnnotations(child);
        }
        return count;
    }
};
