#pragma once
// Step 702: generic/monomorphization intent model.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct RustGenericIntent {
    std::string owner;
    std::vector<std::string> params;
    bool monomorphizedHint = false;
};

class RustGenericIntentModel {
public:
    static std::vector<RustGenericIntent> lower(const std::string& src) {
        std::vector<RustGenericIntent> out;
        auto lines = split(src);
        for (const auto& raw : lines) {
            const auto s = trim(raw);
            if (s.find('<') == std::string::npos || s.find('>') == std::string::npos) continue;
            if (!(startsWith(s, "fn ") || startsWith(s, "struct ") || startsWith(s, "impl<"))) continue;

            RustGenericIntent gi;
            gi.owner = parseOwner(s);
            gi.params = parseParams(s);
            gi.monomorphizedHint = s.find("where") != std::string::npos || s.find(":") != std::string::npos;
            if (!gi.owner.empty() && !gi.params.empty()) out.push_back(std::move(gi));
        }
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.owner < b.owner; });
        return out;
    }

    static nlohmann::json toJson(const std::vector<RustGenericIntent>& v) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& i : v) j.push_back({{"owner", i.owner}, {"params", i.params}, {"monomorphizedHint", i.monomorphizedHint}});
        return j;
    }

private:
    static std::vector<std::string> split(const std::string& s) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s) {
            if (c == '\n') { out.push_back(cur); cur.clear(); } else cur.push_back(c);
        }
        out.push_back(cur);
        return out;
    }

    static std::string trim(const std::string& s) {
        size_t b = 0;
        while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
        size_t e = s.size();
        while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
        return s.substr(b, e - b);
    }

    static bool startsWith(const std::string& s, const std::string& p) {
        return s.rfind(p, 0) == 0;
    }

    static std::string parseOwner(const std::string& s) {
        if (startsWith(s, "impl<")) return "impl";
        auto posSpace = s.find(' ');
        if (posSpace == std::string::npos) return "";
        auto posLt = s.find('<', posSpace + 1);
        if (posLt == std::string::npos) return "";
        return trim(s.substr(posSpace + 1, posLt - (posSpace + 1)));
    }

    static std::vector<std::string> parseParams(const std::string& s) {
        std::vector<std::string> params;
        auto l = s.find('<');
        auto r = s.find('>', l == std::string::npos ? 0 : l + 1);
        if (l == std::string::npos || r == std::string::npos || r <= l + 1) return params;
        std::string body = s.substr(l + 1, r - l - 1);
        std::string cur;
        for (char c : body) {
            if (c == ',') {
                auto t = trim(cur);
                if (!t.empty()) params.push_back(t);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        auto t = trim(cur);
        if (!t.empty()) params.push_back(t);
        return params;
    }
};
