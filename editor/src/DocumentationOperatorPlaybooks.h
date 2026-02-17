#pragma once
// Step 592: Documentation + Operator Playbooks

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct PlaybookDoc {
    std::string docId;
    std::string title;
    std::vector<std::string> sections;
    std::vector<std::string> operatorChecks;
};

class DocumentationOperatorPlaybooks {
public:
    bool addDocument(const std::string& docId,
                     const std::string& title,
                     std::string* error) {
        if (!error) return false;
        error->clear();
        if (docId.empty()) return fail(error, "doc_id_missing");
        if (title.empty()) return fail(error, "doc_title_missing");
        if (docs_.count(docId) != 0) return fail(error, "doc_duplicate");
        docs_[docId] = PlaybookDoc{docId, title, {}, {}};
        return true;
    }

    bool addSection(const std::string& docId,
                    const std::string& section,
                    std::string* error) {
        if (!error) return false;
        error->clear();
        auto* doc = find(docId);
        if (!doc) return fail(error, "doc_missing");
        if (section.empty()) return fail(error, "section_missing");
        for (const auto& s : doc->sections) if (s == section) return fail(error, "section_duplicate");
        doc->sections.push_back(section);
        return true;
    }

    bool addOperatorCheck(const std::string& docId,
                          const std::string& check,
                          std::string* error) {
        if (!error) return false;
        error->clear();
        auto* doc = find(docId);
        if (!doc) return fail(error, "doc_missing");
        if (check.empty()) return fail(error, "check_missing");
        for (const auto& c : doc->operatorChecks) if (c == check) return fail(error, "check_duplicate");
        doc->operatorChecks.push_back(check);
        return true;
    }

    bool isDeploymentReady(const std::string& docId) const {
        const auto* doc = findConst(docId);
        if (!doc) return false;
        return !doc->sections.empty() && !doc->operatorChecks.empty();
    }

    int coverageScore(const std::string& docId) const {
        const auto* doc = findConst(docId);
        if (!doc) return 0;
        int score = 0;
        score += std::min(60, static_cast<int>(doc->sections.size()) * 15);
        score += std::min(40, static_cast<int>(doc->operatorChecks.size()) * 10);
        return score;
    }

    std::vector<PlaybookDoc> allDocuments() const {
        std::vector<PlaybookDoc> out;
        for (const auto& kv : docs_) out.push_back(kv.second);
        std::stable_sort(out.begin(), out.end(), [](const PlaybookDoc& a, const PlaybookDoc& b) {
            return a.docId < b.docId;
        });
        return out;
    }

private:
    std::map<std::string, PlaybookDoc> docs_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    PlaybookDoc* find(const std::string& docId) {
        auto it = docs_.find(docId);
        if (it == docs_.end()) return nullptr;
        return &it->second;
    }

    const PlaybookDoc* findConst(const std::string& docId) const {
        auto it = docs_.find(docId);
        if (it == docs_.end()) return nullptr;
        return &it->second;
    }
};
