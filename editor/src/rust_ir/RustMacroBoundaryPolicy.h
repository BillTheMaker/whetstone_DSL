#pragma once
// Step 705: macro boundary capture + fallback policy.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct RustMacroBoundary {
    std::string macroName;
    bool supported = true;
    std::string fallbackPolicy;
};

class RustMacroBoundaryPolicy {
public:
    static std::vector<RustMacroBoundary> analyze(const std::string& src) {
        std::vector<RustMacroBoundary> out;
        std::vector<std::string> known = {"println", "format", "vec", "panic"};

        size_t p = 0;
        while (true) {
            p = src.find("!", p);
            if (p == std::string::npos) break;
            if (p == 0) { ++p; continue; }
            std::string name = parseNameBackward(src, p - 1);
            if (name.empty()) { ++p; continue; }
            bool supported = std::find(known.begin(), known.end(), name) != known.end();
            out.push_back({name, supported, supported ? "inline_expand" : "manual_review_required"});
            ++p;
        }

        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
            if (a.macroName != b.macroName) return a.macroName < b.macroName;
            return a.fallbackPolicy < b.fallbackPolicy;
        });
        out.erase(std::unique(out.begin(), out.end(), [](const auto& a, const auto& b) {
            return a.macroName == b.macroName && a.fallbackPolicy == b.fallbackPolicy;
        }), out.end());
        return out;
    }

    static nlohmann::json toJson(const std::vector<RustMacroBoundary>& m) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& x : m) {
            j.push_back({{"macroName", x.macroName}, {"supported", x.supported}, {"fallbackPolicy", x.fallbackPolicy}});
        }
        return j;
    }

private:
    static std::string parseNameBackward(const std::string& s, size_t end) {
        if (s.empty()) return "";
        size_t b = end;
        while (b > 0 && (std::isalnum(static_cast<unsigned char>(s[b - 1])) || s[b - 1] == '_')) --b;
        std::string name = s.substr(b, end - b + 1);
        return name;
    }
};
