#pragma once
// SyntaxHighlighterOrg.h helpers for SyntaxHighlighter.

    static void walkOrgSimple(const std::string& source,
                              std::vector<HighlightSpan>& spans) {
        size_t start = 0;
        while (start < source.size()) {
            size_t end = source.find('\n', start);
            if (end == std::string::npos) end = source.size();
            std::string line = source.substr(start, end - start);
            std::string trimmed = line;
            while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == '\n')) {
                trimmed.pop_back();
            }
            if (!trimmed.empty() && trimmed[0] == '*') {
                spans.push_back({(uint32_t)start, (uint32_t)end, TokenCategory::Keyword});
            } else if (trimmed.rfind("#+begin_src", 0) == 0 ||
                       trimmed.rfind("#+end_src", 0) == 0 ||
                       trimmed.rfind("#+", 0) == 0) {
                spans.push_back({(uint32_t)start, (uint32_t)end, TokenCategory::Comment});
            }
            start = end + 1;
        }
    }
