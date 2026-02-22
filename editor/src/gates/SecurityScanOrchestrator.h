#pragma once
// Step 729: security scan orchestrator for generated targets.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct SecurityFinding {
    std::string id;
    std::string severity; // high|medium|low
    std::string file;
};

struct SecurityScanResult {
    std::vector<SecurityFinding> findings;
    int highCount = 0;
};

class SecurityScanOrchestrator {
public:
    static SecurityScanResult run(std::vector<SecurityFinding> findings) {
        std::sort(findings.begin(), findings.end(), [](const SecurityFinding& a, const SecurityFinding& b) {
            if (a.severity != b.severity) return a.severity < b.severity;
            if (a.file != b.file) return a.file < b.file;
            return a.id < b.id;
        });
        SecurityScanResult out;
        out.findings = std::move(findings);
        for (const auto& f : out.findings) if (f.severity == "high") ++out.highCount;
        return out;
    }

    static nlohmann::json toJson(const SecurityScanResult& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& f : r.findings) arr.push_back({{"id", f.id}, {"severity", f.severity}, {"file", f.file}});
        return {{"findings", arr}, {"high_count", r.highCount}};
    }
};
