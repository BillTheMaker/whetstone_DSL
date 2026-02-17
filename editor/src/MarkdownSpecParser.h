#pragma once
// Step 574: Markdown Spec Parser

#include "IntakeTextUtil.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

struct ParsedRequirementItem {
    std::string text;
    std::string sectionTitle;
    std::string anchor;
    int line = 0;
};

struct ParsedSection {
    std::string title;
    std::string anchor;
    int headingLine = 0;
    std::vector<std::string> content;
};

struct ParsedMarkdownSpec {
    std::vector<ParsedSection> sections;
    std::vector<ParsedRequirementItem> goals;
    std::vector<ParsedRequirementItem> constraints;
    std::vector<ParsedRequirementItem> dependencies;
    std::vector<ParsedRequirementItem> acceptanceCriteria;
};

class MarkdownSpecParser {
public:
    static bool parse(const std::string& markdown,
                      ParsedMarkdownSpec* outSpec,
                      std::string* error) {
        if (!outSpec || !error) return false;
        error->clear();
        if (markdown.empty()) {
            *error = "markdown_empty";
            return false;
        }

        ParsedMarkdownSpec spec;
        ParsedSection currentSection;
        bool hasActiveSection = false;

        std::istringstream stream(markdown);
        std::string line;
        int lineNo = 0;
        while (std::getline(stream, line)) {
            ++lineNo;
            if (startsWith(line, "## ")) {
                if (hasActiveSection) spec.sections.push_back(currentSection);
                currentSection = ParsedSection{};
                currentSection.title = trim(line.substr(3));
                currentSection.anchor = makeAnchor(currentSection.title);
                currentSection.headingLine = lineNo;
                hasActiveSection = true;
                continue;
            }

            if (!hasActiveSection) continue;
            currentSection.content.push_back(line);

            if (!startsWith(trimLeft(line), "- ")) continue;
            ParsedRequirementItem item;
            item.text = trim(trimLeft(line).substr(2));
            item.sectionTitle = currentSection.title;
            item.anchor = currentSection.anchor;
            item.line = lineNo;
            if (item.text.empty()) continue;

            const std::string lowerTitle = intakeToLower(currentSection.title);
            if (contains(lowerTitle, "goal")) {
                spec.goals.push_back(item);
            } else if (contains(lowerTitle, "constraint")) {
                spec.constraints.push_back(item);
            } else if (contains(lowerTitle, "dependenc")) {
                spec.dependencies.push_back(item);
            } else if (contains(lowerTitle, "acceptance")) {
                spec.acceptanceCriteria.push_back(item);
            }
        }
        if (hasActiveSection) spec.sections.push_back(currentSection);

        if (spec.sections.empty()) {
            *error = "no_sections_found";
            return false;
        }
        *outSpec = spec;
        return true;
    }

private:
    static bool startsWith(const std::string& text, const std::string& prefix) {
        return text.rfind(prefix, 0) == 0;
    }

    static bool contains(const std::string& text, const std::string& needle) {
        return text.find(needle) != std::string::npos;
    }

    static std::string trimLeft(const std::string& value) {
        std::size_t i = 0;
        while (i < value.size() && std::isspace(static_cast<unsigned char>(value[i]))) ++i;
        return value.substr(i);
    }

    static std::string trim(const std::string& value) {
        std::size_t begin = 0;
        while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) ++begin;
        std::size_t end = value.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
        return value.substr(begin, end - begin);
    }

    static std::string makeAnchor(const std::string& title) {
        std::string anchor = intakeSanitizeToken(title, '-');
        return anchor.empty() ? "section" : anchor;
    }
};
