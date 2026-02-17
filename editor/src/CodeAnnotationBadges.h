#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct AnnotationBadgeInput {
    int line = 0;
    std::string type;
    std::string detail;
    std::string nodeId;
};

struct InlineBadge {
    int line = 0;
    std::string type;
    std::string icon;
    std::string colorHex;
    std::string detail;
    std::string nodeId;
};

struct LineBadgeStack {
    int line = 0;
    std::vector<InlineBadge> badges;
};

class CodeAnnotationBadges {
public:
    static std::vector<InlineBadge> buildBadges(const std::vector<AnnotationBadgeInput>& in) {
        std::vector<InlineBadge> out;
        for (const auto& a : in) {
            InlineBadge b;
            b.line = a.line;
            b.type = a.type;
            b.icon = iconForType(a.type);
            b.colorHex = colorForType(a.type);
            b.detail = a.detail;
            b.nodeId = a.nodeId;
            out.push_back(b);
        }
        std::sort(out.begin(), out.end(),
                  [](const InlineBadge& a, const InlineBadge& b) {
                      if (a.line != b.line) return a.line < b.line;
                      return a.type < b.type;
                  });
        return out;
    }

    static std::vector<LineBadgeStack> stackByLine(const std::vector<InlineBadge>& badges) {
        std::map<int, std::vector<InlineBadge>> grouped;
        for (const auto& b : badges) grouped[b.line].push_back(b);
        std::vector<LineBadgeStack> out;
        for (auto& [line, lineBadges] : grouped) {
            out.push_back({line, lineBadges});
        }
        std::sort(out.begin(), out.end(),
                  [](const LineBadgeStack& a, const LineBadgeStack& b) {
                      return a.line < b.line;
                  });
        return out;
    }

    static std::string expandDetail(const InlineBadge& badge) {
        return badge.type + " [" + badge.nodeId + "]: " + badge.detail;
    }

    static std::string iconForType(const std::string& type) {
        if (type.find("Intent") != std::string::npos) return "I";
        if (type.find("Complexity") != std::string::npos) return "C";
        if (type.find("Risk") != std::string::npos) return "R";
        if (type.find("Contract") != std::string::npos) return "K";
        if (type.find("Review") != std::string::npos) return "V";
        if (type.find("Priority") != std::string::npos) return "P";
        return "A";
    }

    static std::string colorForType(const std::string& type) {
        if (type.find("Intent") != std::string::npos) return "#1ABC9C";
        if (type.find("Complexity") != std::string::npos) return "#F39C12";
        if (type.find("Risk") != std::string::npos) return "#E74C3C";
        if (type.find("Contract") != std::string::npos) return "#3498DB";
        if (type.find("Review") != std::string::npos) return "#9B59B6";
        if (type.find("Priority") != std::string::npos) return "#2ECC71";
        return "#95A5A6";
    }
};
