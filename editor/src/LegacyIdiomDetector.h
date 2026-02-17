#pragma once

// Step 438: Legacy idiom and code-age detection model.

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

struct LegacyIdiomFinding {
    std::string pattern;
    std::string detail;
    int score;
};

struct FunctionLegacyFinding {
    std::string functionName;
    std::vector<LegacyIdiomFinding> findings;
    int legacyScore = 0;
};

struct LegacyAnalysisReport {
    std::string filePath;
    std::string language;
    std::string languageVersion;
    int legacyScore = 0;
    std::vector<LegacyIdiomFinding> findings;
    std::vector<FunctionLegacyFinding> functionFindings;

    bool hasPattern(const std::string& pattern) const {
        for (const auto& f : findings) {
            if (f.pattern == pattern) return true;
        }
        return false;
    }
};

class LegacyIdiomDetector {
public:
    static LegacyAnalysisReport analyzeFile(const std::string& source,
                                            const std::string& language,
                                            const std::string& filePath) {
        LegacyAnalysisReport report;
        report.filePath = filePath;
        report.language = language;
        report.languageVersion = detectLanguageVersion(source, language);

        std::vector<LegacyIdiomFinding> found;
        addIf(hasKnrDeclaration(source), found, "knr-declaration",
              "K&R style function declaration", 3);
        addIf(hasGotoFlow(source), found, "goto-control-flow",
              "goto-heavy control flow", 2);
        addIf(hasDeprecatedCall(source, "gets("), found, "deprecated-api-gets",
              "unsafe API: gets", 3);
        addIf(hasDeprecatedCall(source, "sprintf("), found, "deprecated-api-sprintf",
              "unsafe API: sprintf", 2);
        addIf(hasDeprecatedCall(source, "strcpy("), found, "deprecated-api-strcpy",
              "unsafe API: strcpy", 2);
        addIf(hasPointerArithmetic(source), found, "pointer-arithmetic",
              "raw pointer arithmetic detected", 2);
        addIf(hasManualMemory(source), found, "manual-memory-management",
              "manual memory operations without RAII", 2);

        report.findings = found;
        report.legacyScore = clampScore(sumScore(found));
        report.functionFindings = detectPerFunctionFindings(source, found);
        return report;
    }

    static std::string detectLanguageVersion(const std::string& source,
                                             const std::string& language) {
        std::string l = toLower(language);
        if (l == "c") {
            if (hasKnrDeclaration(source)) return "c89";
            if (source.find("for (int ") != std::string::npos ||
                source.find("for(int ") != std::string::npos) return "c99";
            if (source.find("_Static_assert") != std::string::npos) return "c11";
            return "c99";
        }
        if (l == "cpp" || l == "c++") {
            if (source.find("std::span<") != std::string::npos ||
                source.find("concept ") != std::string::npos) return "cpp20";
            if (source.find("std::optional<") != std::string::npos ||
                source.find("if constexpr") != std::string::npos) return "cpp17";
            if (source.find("std::make_unique<") != std::string::npos ||
                source.find("std::make_shared<") != std::string::npos) return "cpp14";
            if (source.find("auto ") != std::string::npos ||
                source.find("std::unique_ptr<") != std::string::npos ||
                source.find("nullptr") != std::string::npos) return "cpp11";
            return "cpp03";
        }
        if (l == "python") {
            if (source.find("print ") != std::string::npos &&
                source.find("print(") == std::string::npos) return "python2";
            return "python3";
        }
        return "unknown";
    }

private:
    static void addIf(bool cond, std::vector<LegacyIdiomFinding>& out,
                      const std::string& pattern, const std::string& detail, int score) {
        if (!cond) return;
        out.push_back({pattern, detail, score});
    }

    static int sumScore(const std::vector<LegacyIdiomFinding>& findings) {
        int s = 0;
        for (const auto& f : findings) s += f.score;
        return s;
    }

    static int clampScore(int s) {
        if (s < 0) return 0;
        if (s > 10) return 10;
        return s;
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    }

    static bool hasDeprecatedCall(const std::string& src, const std::string& call) {
        return src.find(call) != std::string::npos;
    }

    static bool hasGotoFlow(const std::string& src) {
        return src.find("goto ") != std::string::npos;
    }

    static bool hasKnrDeclaration(const std::string& src) {
        // Heuristic: function signature line "name(a,b)" followed by typed declarations.
        size_t sig = src.find(")\nint ");
        if (sig == std::string::npos) sig = src.find(")\nchar ");
        if (sig == std::string::npos) sig = src.find(")\nfloat ");
        if (sig == std::string::npos) sig = src.find(")\ndouble ");
        if (sig == std::string::npos) sig = src.find(") int ");
        if (sig == std::string::npos) sig = src.find(") char ");
        if (sig == std::string::npos) sig = src.find(") float ");
        if (sig == std::string::npos) sig = src.find(") double ");
        if (sig == std::string::npos) return false;
        size_t paren = src.rfind('(', sig);
        return paren != std::string::npos;
    }

    static bool hasPointerArithmetic(const std::string& src) {
        if (src.find("->") != std::string::npos) return false; // not arithmetic
        return src.find("p+1") != std::string::npos ||
               src.find("p + 1") != std::string::npos ||
               src.find("p + 2") != std::string::npos ||
               src.find("*(p+") != std::string::npos;
    }

    static bool hasManualMemory(const std::string& src) {
        bool hasAlloc = src.find("malloc(") != std::string::npos ||
                        src.find("calloc(") != std::string::npos ||
                        src.find("realloc(") != std::string::npos ||
                        src.find("free(") != std::string::npos;
        bool hasRaii = src.find("std::unique_ptr") != std::string::npos ||
                       src.find("std::shared_ptr") != std::string::npos;
        return hasAlloc && !hasRaii;
    }

    static std::string detectFirstFunctionName(const std::string& source) {
        // Very lightweight name capture for C/C++ style `type name(...){`.
        size_t open = source.find('(');
        if (open == std::string::npos) return "";
        size_t i = open;
        while (i > 0 && std::isspace((unsigned char)source[i - 1])) --i;
        size_t end = i;
        while (i > 0 &&
               (std::isalnum((unsigned char)source[i - 1]) || source[i - 1] == '_')) --i;
        if (end <= i) return "";
        return source.substr(i, end - i);
    }

    static std::vector<FunctionLegacyFinding> detectPerFunctionFindings(
        const std::string& source,
        const std::vector<LegacyIdiomFinding>& allFindings) {
        std::vector<FunctionLegacyFinding> out;
        if (allFindings.empty()) return out;
        std::string fn = detectFirstFunctionName(source);
        if (fn.empty()) return out;
        FunctionLegacyFinding f;
        f.functionName = fn;
        f.findings = allFindings;
        f.legacyScore = clampScore(sumScore(allFindings));
        out.push_back(std::move(f));
        return out;
    }
};
