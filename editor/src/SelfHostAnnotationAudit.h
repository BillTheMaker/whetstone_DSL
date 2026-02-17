#pragma once

// Step 494: Annotate Whetstone's Own Code
// Audits inferred annotations over parsed Whetstone headers.

#include "AnnotationInference.h"
#include "SafetyAuditor.h"
#include "SelfHostCodebaseAudit.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <regex>
#include <set>
#include <string>
#include <vector>

struct FileAnnotationAudit {
    std::string path;
    bool parseSuccess = false;
    int inferredTotal = 0;
    int complexityCount = 0;
    int riskCount = 0;
    int ownerCount = 0;
    int intentCount = 0;
    int tailCallCount = 0;
    std::vector<std::string> warnings;
};

struct SelfAnnotationAuditReport {
    std::vector<FileAnnotationAudit> files;
    int totalFiles = 0;
    int parsedFiles = 0;
    int inferredTotal = 0;
    int complexityCount = 0;
    int riskCount = 0;
    int ownerCount = 0;
    int intentCount = 0;
    int tailCallCount = 0;
    std::vector<std::string> notes;

    double parseRate() const {
        return totalFiles > 0 ? static_cast<double>(parsedFiles) / totalFiles : 1.0;
    }
};

class SelfHostAnnotationAudit {
public:
    static SelfAnnotationAuditReport auditDirectories(const std::vector<std::string>& roots) {
        return auditFiles(SelfHostCodebaseAudit::listHeaderFiles(roots));
    }

    static SelfAnnotationAuditReport auditFiles(const std::vector<std::string>& files) {
        SelfAnnotationAuditReport out;
        out.totalFiles = static_cast<int>(files.size());
        for (const auto& path : files) {
            auto fa = auditFile(path);
            out.files.push_back(fa);
            accumulate(fa, out);
        }
        out.notes.push_back("Annotation inference audit completed");
        return out;
    }

    static FileAnnotationAudit auditSource(const std::string& source,
                                           const std::string& label = "<inline>") {
        FileAnnotationAudit out;
        out.path = label;
        auto parsed = SelfHostHarness::parseSource(source, label);
        out.parseSuccess = parsed.parseSuccess;
        out.warnings = parsed.warnings;
        if (!parsed.parseSuccess || !parsed.ast) return out;

        parsed.ast->targetLanguage = "cpp";
        AnnotationInference inf;
        auto inferred = inf.inferAll(parsed.ast.get());
        out.inferredTotal = static_cast<int>(inferred.size());
        tallyInferred(inferred, out);

        tallyRiskFromSource(source, out);
        tallyOwnerFromSource(source, out);
        tallyIntentFromAst(parsed.ast.get(), out);
        return out;
    }

private:
    static FileAnnotationAudit auditFile(const std::string& path) {
        FileAnnotationAudit out;
        out.path = path;

        auto parsed = SelfHostHarness::parseFile(path);
        out.parseSuccess = parsed.parseSuccess;
        out.warnings = parsed.warnings;
        if (!parsed.parseSuccess || !parsed.ast) return out;

        const std::string source = SelfHostHarness::readFile(path);
        parsed.ast->targetLanguage = "cpp";

        AnnotationInference inf;
        auto inferred = inf.inferAll(parsed.ast.get());
        out.inferredTotal = static_cast<int>(inferred.size());
        tallyInferred(inferred, out);

        tallyRiskFromSource(source, out);
        tallyOwnerFromSource(source, out);
        tallyIntentFromAst(parsed.ast.get(), out);
        return out;
    }

    static void tallyInferred(const std::vector<AnnotationInference::InferredAnnotation>& inferred,
                              FileAnnotationAudit& out) {
        for (const auto& ann : inferred) {
            if (ann.annotationType == "ComplexityAnnotation") out.complexityCount++;
            if (ann.annotationType == "OwnerAnnotation") out.ownerCount++;
            if (ann.annotationType == "TailCallAnnotation") out.tailCallCount++;
        }
    }

    static void tallyRiskFromSource(const std::string& source, FileAnnotationAudit& out) {
        auto safety = SafetyAuditor::audit(source, "cpp", out.path);
        for (const auto& finding : safety.findings) {
            if (finding.riskLevel >= 2) out.riskCount++;
        }
        if (contains(source, "reinterpret_cast") || contains(source, "void*")) {
            out.riskCount++;
        }
    }

    static void tallyOwnerFromSource(const std::string& source, FileAnnotationAudit& out) {
        out.ownerCount += countRegex(source, std::regex(R"(std::unique_ptr<)"));
        out.ownerCount += countRegex(source, std::regex(R"(std::shared_ptr<)"));
    }

    static void tallyIntentFromAst(const Module* ast, FileAnnotationAudit& out) {
        const auto names = SelfHostHarness::getFunctionNames(ast);
        for (const auto& n : names) {
            if (isDescriptiveName(n)) out.intentCount++;
        }
    }

    static bool isDescriptiveName(const std::string& name) {
        static const std::vector<std::string> verbs = {
            "parse", "build", "generate", "validate", "infer", "audit",
            "create", "load", "save", "render", "run", "check"
        };
        std::string lower = toLower(name);
        for (const auto& v : verbs) {
            if (lower.find(v) == 0 || lower.find(v) != std::string::npos) return true;
        }
        return false;
    }

    static int countRegex(const std::string& text, const std::regex& re) {
        auto begin = std::sregex_iterator(text.begin(), text.end(), re);
        auto end = std::sregex_iterator();
        int count = 0;
        for (auto it = begin; it != end; ++it) ++count;
        return count;
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static bool contains(const std::string& src, const std::string& needle) {
        return src.find(needle) != std::string::npos;
    }

    static void accumulate(const FileAnnotationAudit& f, SelfAnnotationAuditReport& out) {
        if (f.parseSuccess) out.parsedFiles++;
        out.inferredTotal += f.inferredTotal;
        out.complexityCount += f.complexityCount;
        out.riskCount += f.riskCount;
        out.ownerCount += f.ownerCount;
        out.intentCount += f.intentCount;
        out.tailCallCount += f.tailCallCount;
    }
};
