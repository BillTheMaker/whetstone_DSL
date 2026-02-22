#pragma once
// Step 699: Rust ownership/borrow graph extractor.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct RustOwnershipRegion {
    std::string id;
    std::string owner;
    std::string kind; // owned, borrowed_mut, borrowed_shared, moved
};

struct RustBorrowEdge {
    std::string from;
    std::string to;
    std::string relation; // borrows, borrows_mut, moves
};

struct RustOwnershipGraph {
    std::vector<RustOwnershipRegion> regions;
    std::vector<RustBorrowEdge> edges;
    std::vector<std::string> diagnostics;
};

class RustOwnershipBorrowExtractor {
public:
    static RustOwnershipGraph extract(const std::string& source) {
        RustOwnershipGraph out;
        std::vector<std::string> lines = splitLines(source);
        int nextId = 1;

        for (const auto& line : lines) {
            const std::string s = trim(line);
            if (s.empty()) continue;

            if (s.find("let ") != std::string::npos && s.find("&mut ") != std::string::npos) {
                std::string lhs = parseLhs(s);
                std::string rhs = parseBorrowTarget(s, "&mut ");
                addRegion(&out, lhs, "borrowed_mut", nextId++);
                out.edges.push_back({rhs, lhs, "borrows_mut"});
                continue;
            }
            if (s.find("let ") != std::string::npos && s.find("&") != std::string::npos) {
                std::string lhs = parseLhs(s);
                std::string rhs = parseBorrowTarget(s, "&");
                addRegion(&out, lhs, "borrowed_shared", nextId++);
                out.edges.push_back({rhs, lhs, "borrows"});
                continue;
            }
            if (s.find("let ") != std::string::npos && s.find("=") != std::string::npos) {
                std::string lhs = parseLhs(s);
                std::string rhs = parseRhsIdent(s);
                addRegion(&out, lhs, "owned", nextId++);
                if (!rhs.empty() && rhs != lhs && !isLiteral(rhs)) {
                    out.edges.push_back({rhs, lhs, "moves"});
                }
                continue;
            }
            if (s.find("drop(") != std::string::npos) {
                out.diagnostics.push_back("drop_call_detected");
            }
        }

        sortDeterministic(&out);
        return out;
    }

    static bool validate(const RustOwnershipGraph& g, std::string* error) {
        if (error) *error = "";
        for (const auto& r : g.regions) {
            if (r.id.empty() || r.owner.empty() || r.kind.empty()) {
                if (error) *error = "region_invalid";
                return false;
            }
        }
        for (const auto& e : g.edges) {
            if (e.from.empty() || e.to.empty() || e.relation.empty()) {
                if (error) *error = "edge_invalid";
                return false;
            }
        }
        return true;
    }

    static nlohmann::json toJson(const RustOwnershipGraph& g) {
        nlohmann::json regions = nlohmann::json::array();
        for (const auto& r : g.regions) {
            regions.push_back({{"id", r.id}, {"owner", r.owner}, {"kind", r.kind}});
        }
        nlohmann::json edges = nlohmann::json::array();
        for (const auto& e : g.edges) {
            edges.push_back({{"from", e.from}, {"to", e.to}, {"relation", e.relation}});
        }
        return {{"regions", regions}, {"edges", edges}, {"diagnostics", g.diagnostics}};
    }

private:
    static std::vector<std::string> splitLines(const std::string& s) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s) {
            if (c == '\n') {
                out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
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

    static std::string parseLhs(const std::string& s) {
        auto posLet = s.find("let ");
        auto posEq = s.find('=');
        if (posLet == std::string::npos || posEq == std::string::npos || posEq <= posLet + 4) return "";
        std::string lhs = trim(s.substr(posLet + 4, posEq - (posLet + 4)));
        if (!lhs.empty() && lhs.back() == ';') lhs.pop_back();
        auto colon = lhs.find(':');
        if (colon != std::string::npos) lhs = trim(lhs.substr(0, colon));
        return lhs;
    }

    static std::string parseBorrowTarget(const std::string& s, const std::string& marker) {
        auto pos = s.find(marker);
        if (pos == std::string::npos) return "";
        std::string rhs = trim(s.substr(pos + marker.size()));
        if (!rhs.empty() && rhs.back() == ';') rhs.pop_back();
        return firstIdent(rhs);
    }

    static std::string parseRhsIdent(const std::string& s) {
        auto posEq = s.find('=');
        if (posEq == std::string::npos) return "";
        std::string rhs = trim(s.substr(posEq + 1));
        if (!rhs.empty() && rhs.back() == ';') rhs.pop_back();
        return firstIdent(rhs);
    }

    static std::string firstIdent(const std::string& s) {
        std::string out;
        bool started = false;
        for (char c : s) {
            if (!started) {
                if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                    started = true;
                    out.push_back(c);
                }
            } else {
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') out.push_back(c);
                else break;
            }
        }
        return out;
    }

    static bool isLiteral(const std::string& ident) {
        if (ident.empty()) return true;
        return std::isdigit(static_cast<unsigned char>(ident[0])) != 0;
    }

    static void addRegion(RustOwnershipGraph* out,
                          const std::string& owner,
                          const std::string& kind,
                          int idNum) {
        if (!out || owner.empty()) return;
        for (const auto& r : out->regions) if (r.owner == owner) return;
        out->regions.push_back({"r" + std::to_string(idNum), owner, kind});
    }

    static void sortDeterministic(RustOwnershipGraph* out) {
        if (!out) return;
        std::sort(out->regions.begin(), out->regions.end(), [](const auto& a, const auto& b) {
            return a.owner < b.owner;
        });
        std::sort(out->edges.begin(), out->edges.end(), [](const auto& a, const auto& b) {
            if (a.from != b.from) return a.from < b.from;
            if (a.to != b.to) return a.to < b.to;
            return a.relation < b.relation;
        });
    }
};
