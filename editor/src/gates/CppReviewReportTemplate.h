#pragma once
// Step 737: reporting templates for C++ review teams.

#include <string>

#include <nlohmann/json.hpp>

class CppReviewReportTemplate {
public:
    static std::string renderMarkdown(const std::string& project,
                                      int highFindings,
                                      bool sanitizerPass,
                                      double worstRegressionPct) {
        return std::string("# Porting Review Report\n\n") +
               "Project: " + project + "\n" +
               "High findings: " + std::to_string(highFindings) + "\n" +
               "Sanitizer pass: " + (sanitizerPass ? "yes" : "no") + "\n" +
               "Worst regression (%): " + std::to_string(worstRegressionPct) + "\n";
    }

    static nlohmann::json toJson(const std::string& markdown) {
        return {{"markdown", markdown}};
    }
};
