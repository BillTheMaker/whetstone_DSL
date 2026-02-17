#pragma once

// Step 439: Safety Audit via Annotations — structured safety analysis
// with CWE mapping and risk-level classification.

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

struct SafetyFinding {
    std::string category;    // e.g. "buffer-overflow", "use-after-free"
    std::string annotation;  // e.g. "@BoundsCheck(unchecked)"
    std::string detail;
    std::string cwe;         // CWE code, e.g. "CWE-120"
    int riskLevel = 0;       // 0=info, 1=low, 2=medium, 3=high, 4=critical
};

struct FunctionSafetyFinding {
    std::string functionName;
    std::vector<SafetyFinding> findings;
    int maxRisk = 0;
};

struct SafetyReport {
    std::string filePath;
    std::string language;
    int overallRisk = 0;     // max risk across all findings
    std::vector<SafetyFinding> findings;
    std::vector<FunctionSafetyFinding> functionFindings;

    bool hasCategory(const std::string& cat) const {
        for (const auto& f : findings)
            if (f.category == cat) return true;
        return false;
    }

    bool hasCwe(const std::string& code) const {
        for (const auto& f : findings)
            if (f.cwe == code) return true;
        return false;
    }

    int countByRisk(int level) const {
        int n = 0;
        for (const auto& f : findings)
            if (f.riskLevel == level) ++n;
        return n;
    }
};

class SafetyAuditor {
public:
    static SafetyReport audit(const std::string& source,
                              const std::string& language,
                              const std::string& filePath) {
        SafetyReport report;
        report.filePath = filePath;
        report.language = toLower(language);

        detectBufferOverflow(source, report);
        detectUseAfterFree(source, report);
        detectNullDeref(source, report);
        detectRaceCondition(source, report);
        detectIntegerOverflow(source, report);

        report.overallRisk = 0;
        for (const auto& f : report.findings)
            report.overallRisk = std::max(report.overallRisk, f.riskLevel);

        report.functionFindings = buildFunctionFindings(source, report.findings);
        return report;
    }

private:
    // --- Buffer overflow (CWE-120, CWE-676) ---
    static void detectBufferOverflow(const std::string& src, SafetyReport& r) {
        // Unchecked array indexing without bounds check
        if (hasPattern(src, "gets(")) {
            r.findings.push_back({
                "buffer-overflow", "@BoundsCheck(unchecked)",
                "gets() has no bounds checking — classic buffer overflow",
                "CWE-120", 4
            });
        }
        if (hasPattern(src, "strcpy(")) {
            r.findings.push_back({
                "buffer-overflow", "@BoundsCheck(unchecked)",
                "strcpy() does not check destination buffer size",
                "CWE-120", 3
            });
        }
        if (hasPattern(src, "sprintf(")) {
            r.findings.push_back({
                "buffer-overflow", "@BoundsCheck(unchecked)",
                "sprintf() does not check buffer bounds",
                "CWE-120", 3
            });
        }
        if (hasPattern(src, "strcat(")) {
            r.findings.push_back({
                "buffer-overflow", "@BoundsCheck(unchecked)",
                "strcat() does not check destination capacity",
                "CWE-120", 3
            });
        }
        // Array access with no visible bounds check
        if (hasUncheckedArrayAccess(src)) {
            r.findings.push_back({
                "buffer-overflow", "@BoundsCheck(unchecked)",
                "array access without visible bounds check",
                "CWE-787", 2
            });
        }
    }

    // --- Use-after-free (CWE-416) ---
    static void detectUseAfterFree(const std::string& src, SafetyReport& r) {
        bool hasFree = hasPattern(src, "free(");
        bool hasMalloc = hasPattern(src, "malloc(") || hasPattern(src, "calloc(");
        bool hasRaii = hasPattern(src, "std::unique_ptr") ||
                       hasPattern(src, "std::shared_ptr");
        if (hasFree && hasMalloc && !hasRaii) {
            r.findings.push_back({
                "use-after-free", "@Owner(Manual)",
                "manual malloc/free without RAII — use-after-free risk",
                "CWE-416", 3
            });
        }
        // Dangling pointer after free — simple heuristic: free() followed by usage
        if (hasFreeFollowedByDeref(src)) {
            r.findings.push_back({
                "use-after-free", "@Lifetime(dangling)",
                "potential dereference after free()",
                "CWE-416", 4
            });
        }
    }

    // --- Null dereference (CWE-476) ---
    static void detectNullDeref(const std::string& src, SafetyReport& r) {
        bool hasNull = hasPattern(src, "NULL") || hasPattern(src, "nullptr");
        bool hasDeref = hasPattern(src, "*") || hasPattern(src, "->");
        bool hasCheck = hasPattern(src, "!= NULL") || hasPattern(src, "!= nullptr") ||
                        hasPattern(src, "== NULL") || hasPattern(src, "== nullptr") ||
                        hasPattern(src, "if (p)") || hasPattern(src, "if(p)");
        if (hasNull && hasDeref && !hasCheck) {
            r.findings.push_back({
                "null-dereference", "@Nullability(nullable)",
                "nullable pointer used without null check",
                "CWE-476", 3
            });
        }
        // Return value of malloc unchecked
        if (hasPattern(src, "malloc(") && !hasPattern(src, "if (") &&
            !hasPattern(src, "if(") && !hasPattern(src, "assert(")) {
            r.findings.push_back({
                "null-dereference", "@Nullability(unchecked-alloc)",
                "malloc() return value not checked for NULL",
                "CWE-476", 2
            });
        }
    }

    // --- Race condition (CWE-362) ---
    static void detectRaceCondition(const std::string& src, SafetyReport& r) {
        bool hasMutableGlobal = hasGlobalMutableState(src);
        bool hasThread = hasPattern(src, "pthread_") || hasPattern(src, "std::thread") ||
                         hasPattern(src, "CreateThread") || hasPattern(src, "thrd_create");
        bool hasSync = hasPattern(src, "mutex") || hasPattern(src, "std::atomic") ||
                       hasPattern(src, "pthread_mutex") || hasPattern(src, "critical_section") ||
                       hasPattern(src, "std::lock_guard") || hasPattern(src, "std::unique_lock");

        if (hasMutableGlobal && hasThread && !hasSync) {
            r.findings.push_back({
                "race-condition", "@Sync(none)",
                "shared mutable state with threading but no synchronization",
                "CWE-362", 3
            });
        }
        if (hasMutableGlobal && !hasSync && !hasThread) {
            // Weaker signal — global mutable state without protection
            r.findings.push_back({
                "race-condition", "@Sync(unprotected-global)",
                "global mutable state without synchronization primitives",
                "CWE-362", 1
            });
        }
    }

    // --- Integer overflow (CWE-190) ---
    static void detectIntegerOverflow(const std::string& src, SafetyReport& r) {
        bool hasArith = hasPattern(src, " + ") || hasPattern(src, " * ") ||
                        hasPattern(src, "+=") || hasPattern(src, "*=");
        bool hasCheck = hasPattern(src, "INT_MAX") || hasPattern(src, "UINT_MAX") ||
                        hasPattern(src, "std::numeric_limits") ||
                        hasPattern(src, "__builtin_add_overflow") ||
                        hasPattern(src, "__builtin_mul_overflow") ||
                        hasPattern(src, "SafeInt") || hasPattern(src, "CheckedInt");
        bool hasIntType = hasPattern(src, "int ") || hasPattern(src, "size_t ") ||
                          hasPattern(src, "unsigned ");
        if (hasArith && hasIntType && !hasCheck) {
            r.findings.push_back({
                "integer-overflow", "@Overflow(unchecked)",
                "integer arithmetic without overflow check",
                "CWE-190", 2
            });
        }
    }

    // --- Helpers ---
    static bool hasPattern(const std::string& src, const std::string& pat) {
        return src.find(pat) != std::string::npos;
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    }

    static bool hasUncheckedArrayAccess(const std::string& src) {
        // Heuristic: `buf[` present but no `if (i <` or `assert` or `size()`
        if (src.find('[') == std::string::npos) return false;
        // Look for array subscript without bounds checking
        bool hasSubscript = false;
        for (size_t i = 0; i < src.size(); ++i) {
            if (src[i] == '[' && i > 0 && (std::isalnum((unsigned char)src[i-1]) || src[i-1] == '_')) {
                hasSubscript = true;
                break;
            }
        }
        if (!hasSubscript) return false;
        bool hasBoundsCheck = hasPattern(src, ".size()") ||
                              hasPattern(src, ".length()") ||
                              hasPattern(src, "< sizeof") ||
                              hasPattern(src, "assert(");
        return !hasBoundsCheck;
    }

    static bool hasGlobalMutableState(const std::string& src) {
        // Heuristic: line starts with a non-const type followed by identifier at file scope
        // Simplified: look for `static int`, `static char*`, `int g_`, or global assignments
        return hasPattern(src, "static int ") ||
               hasPattern(src, "static char ") ||
               hasPattern(src, "int g_") ||
               hasPattern(src, "volatile ");
    }

    static bool hasFreeFollowedByDeref(const std::string& src) {
        size_t freePos = src.find("free(");
        if (freePos == std::string::npos) return false;
        // Check for pointer usage after free()
        size_t afterFree = src.find(';', freePos);
        if (afterFree == std::string::npos) return false;
        std::string rest = src.substr(afterFree);
        // Look for dereference of the freed pointer
        return hasPattern(rest, "->") || hasPattern(rest, "*p");
    }

    static std::string detectFirstFunctionName(const std::string& source) {
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

    static std::vector<FunctionSafetyFinding> buildFunctionFindings(
        const std::string& source,
        const std::vector<SafetyFinding>& allFindings) {
        std::vector<FunctionSafetyFinding> out;
        if (allFindings.empty()) return out;
        std::string fn = detectFirstFunctionName(source);
        if (fn.empty()) return out;
        FunctionSafetyFinding f;
        f.functionName = fn;
        f.findings = allFindings;
        f.maxRisk = 0;
        for (const auto& sf : allFindings)
            f.maxRisk = std::max(f.maxRisk, sf.riskLevel);
        out.push_back(std::move(f));
        return out;
    }
};
