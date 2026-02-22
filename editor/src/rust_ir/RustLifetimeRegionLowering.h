#pragma once
// Step 700: lifetime region lowering.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct RustLifetimeRegion {
    std::string name;   // 'a, 'static
    std::string scope;  // fn, impl, type
};

class RustLifetimeRegionLowering {
public:
    static std::vector<RustLifetimeRegion> lower(const std::string& source) {
        std::vector<RustLifetimeRegion> out;
        for (size_t i = 0; i < source.size(); ++i) {
            if (source[i] != '\'') continue;
            size_t j = i + 1;
            std::string ident;
            while (j < source.size()) {
                char c = source[j];
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                    ident.push_back(c);
                    ++j;
                } else {
                    break;
                }
            }
            if (ident.empty()) continue;
            std::string full = "'" + ident;
            if (!contains(out, full)) {
                out.push_back({full, inferScope(source, i)});
            }
            i = j;
        }
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.name < b.name; });
        return out;
    }

    static nlohmann::json toJson(const std::vector<RustLifetimeRegion>& regs) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& r : regs) j.push_back({{"name", r.name}, {"scope", r.scope}});
        return j;
    }

private:
    static bool contains(const std::vector<RustLifetimeRegion>& out, const std::string& name) {
        for (const auto& r : out) if (r.name == name) return true;
        return false;
    }

    static std::string inferScope(const std::string& source, size_t pos) {
        std::string prefix = source.substr(0, pos);
        if (prefix.rfind("impl", std::string::npos) != std::string::npos) return "impl";
        if (prefix.rfind("struct", std::string::npos) != std::string::npos) return "type";
        return "fn";
    }
};
