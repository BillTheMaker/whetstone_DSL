#pragma once

// Step 484: Prior-Step Context Injector
// Builds a condensed API/context brief from prior step headers.

#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct ApiMethodSummary {
    std::string signature;
};

struct ApiTypeSummary {
    std::string kind; // class | struct
    std::string name;
    std::vector<ApiMethodSummary> methods;
};

struct StepContext {
    std::vector<ApiTypeSummary> priorApis;
    std::map<std::string, std::vector<std::string>> includeDependencies;
    std::vector<std::string> namingPatterns;
    std::vector<std::string> warnings;

    std::string condensedBrief() const {
        std::ostringstream out;
        out << "types:";
        for (const auto& t : priorApis) out << " " << t.name;
        out << " | patterns:";
        for (const auto& p : namingPatterns) out << " " << p;
        return out.str();
    }
};

class PriorStepContextInjector {
public:
    static StepContext build(const std::vector<std::string>& headerPaths) {
        StepContext out;
        std::set<std::string> patterns;

        for (const auto& p : headerPaths) {
            std::string text = readAll(p);
            if (text.empty()) {
                out.warnings.push_back("Missing or unreadable header: " + p);
                continue;
            }

            out.includeDependencies[p] = parseIncludes(text);
            auto types = parseTypesAndMethods(text);
            for (const auto& t : types) {
                out.priorApis.push_back(t);
                if (endsWith(t.name, "Report")) patterns.insert("Suffix:Report");
                if (endsWith(t.name, "Spec")) patterns.insert("Suffix:Spec");
                if (endsWith(t.name, "State")) patterns.insert("Suffix:State");
            }
        }

        out.namingPatterns.assign(patterns.begin(), patterns.end());
        return out;
    }

private:
    static std::string readAll(const std::string& path) {
        std::ifstream in(path);
        if (!in.is_open()) return "";
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    static bool endsWith(const std::string& v, const std::string& suffix) {
        return v.size() >= suffix.size() &&
               v.compare(v.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    static std::vector<std::string> parseIncludes(const std::string& text) {
        std::vector<std::string> out;
        std::regex incRe(R"inc(#include\s+"([^"]+)")inc");
        auto begin = std::sregex_iterator(text.begin(), text.end(), incRe);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            out.push_back((*it)[1].str());
        }
        return out;
    }

    static std::vector<ApiTypeSummary> parseTypesAndMethods(const std::string& text) {
        std::vector<ApiTypeSummary> out;
        std::regex typeRe(R"(\b(class|struct)\s+([A-Za-z_][A-Za-z0-9_]*))");
        auto begin = std::sregex_iterator(text.begin(), text.end(), typeRe);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            ApiTypeSummary t;
            t.kind = (*it)[1].str();
            t.name = (*it)[2].str();
            out.push_back(std::move(t));
        }

        std::regex methodRe(
            R"((?:static\s+)?[A-Za-z_][A-Za-z0-9_:<>\s,&\*]*\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*\)\s*(?:const\s*)?(?:\{|;))");
        auto mBegin = std::sregex_iterator(text.begin(), text.end(), methodRe);
        auto mEnd = std::sregex_iterator();

        std::vector<ApiMethodSummary> methods;
        for (auto it = mBegin; it != mEnd; ++it) {
            ApiMethodSummary m;
            m.signature = std::regex_replace((*it).str(), std::regex(R"(\s+)"), " ");
            if (!m.signature.empty() &&
                (m.signature.back() == '{' || m.signature.back() == ';')) {
                m.signature.pop_back();
            }
            methods.push_back(std::move(m));
        }
        if (!out.empty()) {
            for (auto& t : out) t.methods = methods;
        }
        return out;
    }
};
