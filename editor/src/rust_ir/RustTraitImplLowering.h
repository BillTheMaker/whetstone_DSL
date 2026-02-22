#pragma once
// Step 701: trait + impl lowering.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct RustTraitDecl {
    std::string traitName;
    std::vector<std::string> methods;
};

struct RustImplDecl {
    std::string traitName;
    std::string forType;
};

struct RustTraitImplModel {
    std::vector<RustTraitDecl> traits;
    std::vector<RustImplDecl> impls;
};

class RustTraitImplLowering {
public:
    static RustTraitImplModel lower(const std::string& src) {
        RustTraitImplModel out;
        auto lines = splitLines(src);
        for (size_t i = 0; i < lines.size(); ++i) {
            auto s = trim(lines[i]);
            if (startsWith(s, "trait ")) {
                RustTraitDecl td;
                td.traitName = parseAfterKeyword(s, "trait");
                for (size_t j = i + 1; j < lines.size(); ++j) {
                    auto m = trim(lines[j]);
                    if (m.find('}') != std::string::npos) break;
                    if (startsWith(m, "fn ")) td.methods.push_back(parseAfterKeyword(m, "fn"));
                }
                out.traits.push_back(std::move(td));
            } else if (startsWith(s, "impl ") && s.find(" for ") != std::string::npos) {
                RustImplDecl id;
                auto body = s.substr(5);
                auto pos = body.find(" for ");
                id.traitName = trim(body.substr(0, pos));
                id.forType = firstToken(trim(body.substr(pos + 5)));
                out.impls.push_back(std::move(id));
            }
        }
        std::sort(out.traits.begin(), out.traits.end(), [](const auto& a, const auto& b) {
            return a.traitName < b.traitName;
        });
        std::sort(out.impls.begin(), out.impls.end(), [](const auto& a, const auto& b) {
            if (a.traitName != b.traitName) return a.traitName < b.traitName;
            return a.forType < b.forType;
        });
        return out;
    }

    static nlohmann::json toJson(const RustTraitImplModel& m) {
        nlohmann::json jTraits = nlohmann::json::array();
        for (const auto& t : m.traits) jTraits.push_back({{"trait", t.traitName}, {"methods", t.methods}});
        nlohmann::json jImpls = nlohmann::json::array();
        for (const auto& i : m.impls) jImpls.push_back({{"trait", i.traitName}, {"for", i.forType}});
        return {{"traits", jTraits}, {"impls", jImpls}};
    }

private:
    static std::vector<std::string> splitLines(const std::string& s) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s) {
            if (c == '\n') { out.push_back(cur); cur.clear(); }
            else cur.push_back(c);
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

    static std::string parseAfterKeyword(const std::string& s, const std::string& kw) {
        auto pos = s.find(kw);
        if (pos == std::string::npos) return "";
        std::string tail = trim(s.substr(pos + kw.size()));
        return firstToken(tail);
    }

    static std::string firstToken(const std::string& s) {
        std::string out;
        for (char c : s) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '\'') out.push_back(c);
            else break;
        }
        return out;
    }
};
