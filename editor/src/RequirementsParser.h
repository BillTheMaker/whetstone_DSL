#pragma once

#include "IntakeTextUtil.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

struct ParsedRequirement {
    std::string text;
    std::string sourceText;
    int         sourceLine = 0;
    std::string sectionAnchor;
    std::string sectionTitle;
    bool        isField = false;
    std::string fieldName;
    std::string fieldType;
};

class RequirementsParser {
public:
    static std::vector<ParsedRequirement> parse(const std::string& markdown) {
        std::vector<ParsedRequirement> out;
        if (markdown.empty()) return out;

        std::istringstream stream(markdown);
        std::string line;
        std::string sectionTitle;
        std::string sectionAnchor;
        int lineNo = 0;

        while (std::getline(stream, line)) {
            ++lineNo;
            std::string trimmed = trim(line);
            if (isHeading(trimmed)) {
                sectionTitle = trimHeading(trimmed);
                sectionAnchor = anchorize(sectionTitle);
                continue;
            }

            if (!isRequirementHeading(sectionAnchor)) continue;

            if (isBullet(trimmed)) {
                std::string bulletText = trim(trimmed.substr(2));
                if (bulletText.empty()) continue;
                if (tryParseFieldList(bulletText, lineNo, sectionTitle, sectionAnchor, &out)) continue;
                ParsedRequirement req;
                req.text = normalizeText(bulletText);
                req.sourceText = bulletText;
                req.sourceLine = lineNo;
                req.sectionAnchor = sectionAnchor;
                req.sectionTitle = sectionTitle;
                out.push_back(req);
            } else if (isProseLine(trimmed)) {
                ParsedRequirement req;
                req.text = normalizeText(trimmed);
                req.sourceText = trimmed;
                req.sourceLine = lineNo;
                req.sectionAnchor = sectionAnchor;
                req.sectionTitle = sectionTitle;
                out.push_back(req);
            }
        }

        return out;
    }

    static std::string anchorize(const std::string& heading) {
        std::string out;
        bool prevDash = false;
        for (unsigned char ch : heading) {
            if (std::isalnum(ch)) {
                out.push_back(static_cast<char>(std::tolower(ch)));
                prevDash = false;
            } else if (!out.empty() && !prevDash) {
                out.push_back('-');
                prevDash = true;
            }
        }
        while (!out.empty() && out.back() == '-') out.pop_back();
        return out;
    }

    static bool isRequirementHeading(const std::string& anchor) {
        return anchor == "requirements" ||
               anchor == "features" ||
               anchor == "behavior" ||
               anchor == "behaviour" ||
               anchor == "interface" ||
               anchor == "api";
    }

private:
    static std::string trim(const std::string& value) {
        std::size_t begin = 0;
        while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) ++begin;
        std::size_t end = value.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
        return value.substr(begin, end - begin);
    }

    static std::string normalizeText(const std::string& text) {
        return intakeCompactLowerAlnumSpaces(text);
    }

    static bool isHeading(const std::string& line) {
        if (line.size() < 2) return false;
        if (line[0] != '#') return false;
        std::size_t hashes = 0;
        while (hashes < line.size() && line[hashes] == '#') ++hashes;
        return hashes > 0 && hashes < line.size() && line[hashes] == ' ';
    }

    static std::string trimHeading(const std::string& headingLine) {
        std::size_t i = 0;
        while (i < headingLine.size() && headingLine[i] == '#') ++i;
        if (i < headingLine.size() && headingLine[i] == ' ') ++i;
        return trim(headingLine.substr(i));
    }

    static bool isBullet(const std::string& line) {
        if (line.size() < 3) return false;
        return (line[0] == '-' || line[0] == '*') && line[1] == ' ';
    }

    static bool isProseLine(const std::string& trimmed) {
        if (trimmed.size() < 10) return false;
        if (isHeading(trimmed)) return false;
        if (trimmed[0] == '`' || trimmed[0] == '|' || trimmed[0] == '>') return false;
        return true;
    }

    static std::string maybeFieldType(const std::string& token) {
        std::size_t l = token.find('(');
        std::size_t r = token.rfind(')');
        if (l == std::string::npos || r == std::string::npos || r <= l) return "";
        return trim(token.substr(l + 1, r - l - 1));
    }

    static std::string fieldNameFromToken(const std::string& token) {
        std::string work = trim(token);
        std::size_t l = work.find('(');
        if (l != std::string::npos) work = trim(work.substr(0, l));
        return work;
    }

    static bool tryParseFieldList(const std::string& bulletText,
                                  int sourceLine,
                                  const std::string& sectionTitle,
                                  const std::string& sectionAnchor,
                                  std::vector<ParsedRequirement>* out) {
        std::size_t colon = bulletText.find(':');
        if (colon == std::string::npos) return false;

        std::string prefix = trim(bulletText.substr(0, colon));
        std::string lowerPrefix = intakeToLower(prefix);
        if (lowerPrefix.find(" has") == std::string::npos && lowerPrefix != "has") return false;

        std::string rhs = trim(bulletText.substr(colon + 1));
        if (rhs.find(',') == std::string::npos) return false;
        std::stringstream ss(rhs);
        std::string token;
        bool emitted = false;
        while (std::getline(ss, token, ',')) {
            std::string name = fieldNameFromToken(token);
            if (name.empty()) continue;

            ParsedRequirement req;
            req.isField = true;
            req.fieldName = name;
            req.fieldType = maybeFieldType(token);
            req.sourceText = trim(token);
            req.text = normalizeText(prefix + " " + name + (req.fieldType.empty() ? "" : " " + req.fieldType));
            req.sourceLine = sourceLine;
            req.sectionAnchor = sectionAnchor;
            req.sectionTitle = sectionTitle;
            out->push_back(req);
            emitted = true;
        }

        return emitted;
    }
};
