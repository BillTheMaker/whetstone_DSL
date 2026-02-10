#pragma once

#include <algorithm>
#include <cctype>
#include <regex>
#include <string>
#include <vector>

struct SearchOptions {
    bool matchCase = false;
    bool wholeWord = false;
    bool useRegex = false;
};

struct SearchMatch {
    int start = 0;
    int end = 0;
    int line = 0;
    int col = 0;
    std::string lineText;
    std::string matchText;
};

namespace SearchUtils {
inline bool isWordChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

inline bool isWholeWord(const std::string& text, int start, int end) {
    bool leftOk = (start <= 0) || !isWordChar(text[start - 1]);
    bool rightOk = (end >= (int)text.size()) || !isWordChar(text[end]);
    return leftOk && rightOk;
}

inline std::vector<int> buildLineStarts(const std::string& text) {
    std::vector<int> starts;
    starts.reserve(128);
    starts.push_back(0);
    for (int i = 0; i < (int)text.size(); ++i) {
        if (text[i] == '\n') starts.push_back(i + 1);
    }
    return starts;
}

inline void lineColFromPos(const std::vector<int>& lineStarts, int pos, int& outLine, int& outCol) {
    auto it = std::upper_bound(lineStarts.begin(), lineStarts.end(), pos);
    int line = (int)(it - lineStarts.begin()) - 1;
    if (line < 0) line = 0;
    outLine = line;
    outCol = pos - lineStarts[line];
}

inline std::string lineTextAt(const std::string& text, const std::vector<int>& lineStarts, int line) {
    if (line < 0 || line >= (int)lineStarts.size()) return {};
    int start = lineStarts[line];
    int end = (line + 1 < (int)lineStarts.size()) ? lineStarts[line + 1] - 1 : (int)text.size();
    if (end < start) end = start;
    return text.substr(start, end - start);
}

inline std::vector<SearchMatch> collectMatches(const std::string& text,
                                               const std::string& query,
                                               const SearchOptions& options,
                                               int rangeStart,
                                               int rangeEnd) {
    std::vector<SearchMatch> matches;
    if (query.empty()) return matches;
    rangeStart = std::max(0, std::min(rangeStart, (int)text.size()));
    rangeEnd = std::max(rangeStart, std::min(rangeEnd, (int)text.size()));

    const std::vector<int> lineStarts = buildLineStarts(text);
    const std::string slice = text.substr(rangeStart, rangeEnd - rangeStart);

    if (options.useRegex) {
        try {
            auto flags = std::regex_constants::ECMAScript;
            if (!options.matchCase) flags |= std::regex_constants::icase;
            std::regex re(query, flags);
            for (auto it = std::sregex_iterator(slice.begin(), slice.end(), re);
                 it != std::sregex_iterator(); ++it) {
                int start = rangeStart + (int)it->position();
                int end = start + (int)it->length();
                if (options.wholeWord && !isWholeWord(text, start, end)) continue;
                SearchMatch m;
                m.start = start;
                m.end = end;
                lineColFromPos(lineStarts, start, m.line, m.col);
                m.lineText = lineTextAt(text, lineStarts, m.line);
                m.matchText = it->str();
                matches.push_back(std::move(m));
            }
        } catch (...) {
            return matches;
        }
        return matches;
    }

    std::string haystack = slice;
    std::string needle = query;
    if (!options.matchCase) {
        std::transform(haystack.begin(), haystack.end(), haystack.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        std::transform(needle.begin(), needle.end(), needle.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
    }

    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        int start = rangeStart + (int)pos;
        int end = start + (int)needle.size();
        if (options.wholeWord && !isWholeWord(text, start, end)) {
            pos += needle.size();
            continue;
        }
        SearchMatch m;
        m.start = start;
        m.end = end;
        lineColFromPos(lineStarts, start, m.line, m.col);
        m.lineText = lineTextAt(text, lineStarts, m.line);
        m.matchText = text.substr(start, end - start);
        matches.push_back(std::move(m));
        pos += needle.size();
    }
    return matches;
}

inline bool regexGroupsForFirstMatch(const std::string& text,
                                     const std::string& query,
                                     const SearchOptions& options,
                                     int rangeStart,
                                     int rangeEnd,
                                     std::vector<std::string>& outGroups) {
    outGroups.clear();
    if (query.empty()) return false;
    rangeStart = std::max(0, std::min(rangeStart, (int)text.size()));
    rangeEnd = std::max(rangeStart, std::min(rangeEnd, (int)text.size()));
    const std::string slice = text.substr(rangeStart, rangeEnd - rangeStart);
    try {
        auto flags = std::regex_constants::ECMAScript;
        if (!options.matchCase) flags |= std::regex_constants::icase;
        std::regex re(query, flags);
        std::smatch match;
        if (!std::regex_search(slice, match, re)) return false;
        for (size_t i = 1; i < match.size(); ++i) {
            outGroups.push_back(match.str(i));
        }
        return true;
    } catch (...) {
        return false;
    }
}

inline std::string applyReplacement(const std::string& matchText,
                                    const std::string& query,
                                    const std::string& replacement,
                                    const SearchOptions& options) {
    if (!options.useRegex) return replacement;
    try {
        auto flags = std::regex_constants::ECMAScript;
        if (!options.matchCase) flags |= std::regex_constants::icase;
        std::regex re(query, flags);
        return std::regex_replace(matchText, re, replacement,
                                  std::regex_constants::format_first_only);
    } catch (...) {
        return replacement;
    }
}

inline std::string replaceAll(const std::string& text,
                              const std::string& query,
                              const std::string& replacement,
                              const SearchOptions& options,
                              int rangeStart,
                              int rangeEnd,
                              int& outCount) {
    outCount = 0;
    if (query.empty()) return text;
    rangeStart = std::max(0, std::min(rangeStart, (int)text.size()));
    rangeEnd = std::max(rangeStart, std::min(rangeEnd, (int)text.size()));
    std::string result = text;

    if (options.useRegex) {
        try {
            auto flags = std::regex_constants::ECMAScript;
            if (!options.matchCase) flags |= std::regex_constants::icase;
            std::regex re(query, flags);
            const std::string slice = text.substr(rangeStart, rangeEnd - rangeStart);
            for (auto it = std::sregex_iterator(slice.begin(), slice.end(), re);
                 it != std::sregex_iterator(); ++it) {
                ++outCount;
            }
            std::string replaced = std::regex_replace(slice, re, replacement);
            result.replace(rangeStart, rangeEnd - rangeStart, replaced);
            return result;
        } catch (...) {
            return text;
        }
    }

    SearchOptions opts = options;
    opts.useRegex = false;
    std::vector<SearchMatch> matches = collectMatches(text, query, opts, rangeStart, rangeEnd);
    if (matches.empty()) return text;
    for (int i = (int)matches.size() - 1; i >= 0; --i) {
        const auto& m = matches[i];
        result.replace(m.start, m.end - m.start, replacement);
        ++outCount;
    }
    return result;
}
} // namespace SearchUtils
