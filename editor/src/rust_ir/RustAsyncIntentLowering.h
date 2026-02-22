#pragma once
// Step 704: async/await state intent lowering.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct RustAsyncIntent {
    std::string functionName;
    int awaitCount = 0;
    bool returnsFuture = false;
};

class RustAsyncIntentLowering {
public:
    static std::vector<RustAsyncIntent> lower(const std::string& src) {
        std::vector<RustAsyncIntent> out;
        auto lines = split(src);
        for (const auto& raw : lines) {
            std::string s = trim(raw);
            if (s.rfind("async fn ", 0) != 0) continue;
            RustAsyncIntent i;
            i.functionName = parseName(s.substr(9));
            i.awaitCount = countAwait(src);
            i.returnsFuture = true;
            out.push_back(std::move(i));
        }
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.functionName < b.functionName; });
        return out;
    }

    static nlohmann::json toJson(const std::vector<RustAsyncIntent>& v) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& i : v) j.push_back({{"functionName", i.functionName}, {"awaitCount", i.awaitCount}, {"returnsFuture", i.returnsFuture}});
        return j;
    }

private:
    static int countAwait(const std::string& src) {
        int n = 0;
        size_t p = 0;
        while (true) {
            p = src.find(".await", p);
            if (p == std::string::npos) break;
            ++n;
            p += 6;
        }
        return n;
    }

    static std::string parseName(const std::string& tail) {
        std::string out;
        for (char c : tail) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') out.push_back(c);
            else break;
        }
        return out;
    }

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
};
