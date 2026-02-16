#pragma once
// Step 359: Find/replace bar model with in-file and project search.

#include <string>
#include <utility>
#include <vector>

struct FindMatch {
    int start = -1;
    int end = -1;
    int line = 1;
    int column = 1;
};

struct ProjectFindResult {
    std::string filePath;
    int line = 1;
    int column = 1;
    std::string preview;
};

class FindReplaceBar {
public:
    void openFind() {
        visible_ = true;
        replaceMode_ = false;
    }

    void openReplace() {
        visible_ = true;
        replaceMode_ = true;
    }

    void close() {
        visible_ = false;
    }

    bool isVisible() const { return visible_; }
    bool isReplaceMode() const { return replaceMode_; }

    void setQuery(const std::string& query, const std::string& text) {
        query_ = query;
        rebuildMatches(text);
    }

    void setReplaceText(const std::string& repl) { replaceText_ = repl; }

    const std::string& query() const { return query_; }
    const std::string& replaceText() const { return replaceText_; }

    const std::vector<FindMatch>& matches() const { return matches_; }

    int matchCount() const { return static_cast<int>(matches_.size()); }
    int currentIndex() const { return currentIndex_; }

    bool hasHighlights() const { return !matches_.empty(); }

    std::string matchCounterText() const {
        if (matches_.empty()) return "0 of 0";
        return std::to_string(currentIndex_ + 1) + " of " + std::to_string(matches_.size());
    }

    void nextMatch() {
        if (matches_.empty()) return;
        currentIndex_ = (currentIndex_ + 1) % static_cast<int>(matches_.size());
    }

    void prevMatch() {
        if (matches_.empty()) return;
        currentIndex_ = (currentIndex_ - 1 + static_cast<int>(matches_.size())) %
                        static_cast<int>(matches_.size());
    }

    bool replaceCurrent(std::string& text) {
        if (matches_.empty() || currentIndex_ < 0 || currentIndex_ >= (int)matches_.size()) return false;
        const auto m = matches_[currentIndex_];
        text.replace(m.start, m.end - m.start, replaceText_);
        rebuildMatches(text);
        return true;
    }

    int replaceAll(std::string& text) {
        if (query_.empty()) return 0;
        int replaced = 0;
        size_t pos = 0;
        while (true) {
            size_t found = text.find(query_, pos);
            if (found == std::string::npos) break;
            text.replace(found, query_.size(), replaceText_);
            pos = found + replaceText_.size();
            replaced++;
        }
        rebuildMatches(text);
        return replaced;
    }

    std::vector<ProjectFindResult> searchProject(
        const std::vector<std::pair<std::string, std::string>>& files) const {
        std::vector<ProjectFindResult> out;
        if (query_.empty()) return out;
        for (const auto& [path, content] : files) {
            int line = 1;
            int col = 1;
            for (size_t i = 0; i < content.size(); ++i) {
                if (i + query_.size() <= content.size() &&
                    content.compare(i, query_.size(), query_) == 0) {
                    ProjectFindResult r;
                    r.filePath = path;
                    r.line = line;
                    r.column = col;
                    size_t previewStart = i > 10 ? i - 10 : 0;
                    size_t previewLen = std::min<size_t>(40, content.size() - previewStart);
                    r.preview = content.substr(previewStart, previewLen);
                    out.push_back(r);
                }
                if (content[i] == '\n') {
                    line++;
                    col = 1;
                } else {
                    col++;
                }
            }
        }
        return out;
    }

private:
    void rebuildMatches(const std::string& text) {
        matches_.clear();
        currentIndex_ = 0;
        if (query_.empty()) {
            currentIndex_ = -1;
            return;
        }
        int line = 1;
        int col = 1;
        for (size_t i = 0; i + query_.size() <= text.size(); ++i) {
            if (text.compare(i, query_.size(), query_) == 0) {
                FindMatch m;
                m.start = static_cast<int>(i);
                m.end = static_cast<int>(i + query_.size());
                m.line = line;
                m.column = col;
                matches_.push_back(m);
            }
            if (text[i] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
        }
        if (matches_.empty()) currentIndex_ = -1;
    }

    bool visible_ = false;
    bool replaceMode_ = false;
    std::string query_;
    std::string replaceText_;
    std::vector<FindMatch> matches_;
    int currentIndex_ = -1;
};
