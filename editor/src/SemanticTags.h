#pragma once
// Step 193: Semantic annotation tags for libraries.

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <nlohmann/json.hpp>

class SemanticTags {
public:
    using TagList = std::vector<std::string>;

    const TagList& vocabulary() const { return vocab_; }

    void load() {
        if (loaded_) return;
        loaded_ = true;
        tags_.clear();
        std::filesystem::path path = defaultPath();
        std::ifstream in(path);
        if (!in.is_open()) return;
        nlohmann::json j;
        try { in >> j; } catch (...) { return; }
        if (!j.is_object()) return;
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (!it.value().is_object()) continue;
            for (auto symIt = it.value().begin(); symIt != it.value().end(); ++symIt) {
                TagList tags;
                if (symIt.value().is_array()) {
                    for (const auto& tag : symIt.value()) {
                        if (tag.is_string()) tags.push_back(normalizeTag(tag.get<std::string>()));
                    }
                }
                if (!tags.empty()) tags_[it.key()][symIt.key()] = tags;
            }
        }
    }

    void save() const {
        std::filesystem::path path = defaultPath();
        std::filesystem::create_directories(path.parent_path());
        nlohmann::json j = nlohmann::json::object();
        for (const auto& [lib, symMap] : tags_) {
            nlohmann::json symObj = nlohmann::json::object();
            for (const auto& [sym, tags] : symMap) {
                symObj[sym] = tags;
            }
            j[lib] = symObj;
        }
        std::ofstream out(path);
        if (!out.is_open()) return;
        out << j.dump(2);
    }

    void setTags(const std::string& library,
                 const std::string& symbol,
                 const TagList& tags) {
        load();
        TagList normalized;
        for (const auto& t : tags) normalized.push_back(normalizeTag(t));
        tags_[library][symbol] = dedupe(normalized);
    }

    TagList tagsForLibrary(const std::string& library) {
        load();
        return dedupe(autoTagsForLibrary(library));
    }

    TagList tagsForSymbol(const std::string& library,
                          const std::string& symbol,
                          const std::string& doc = "") {
        load();
        TagList out;
        TagList libTags = autoTagsForLibrary(library);
        out.insert(out.end(), libTags.begin(), libTags.end());
        auto libIt = tags_.find(library);
        if (libIt != tags_.end()) {
            auto symIt = libIt->second.find(symbol);
            if (symIt != libIt->second.end()) {
                out.insert(out.end(), symIt->second.begin(), symIt->second.end());
            }
        }
        TagList inferred = inferTagsFromName(symbol);
        if (!doc.empty()) {
            TagList docInferred = inferTagsFromName(doc);
            inferred.insert(inferred.end(), docInferred.begin(), docInferred.end());
        }
        out.insert(out.end(), inferred.begin(), inferred.end());
        return dedupe(out);
    }

private:
    bool loaded_ = false;
    std::unordered_map<std::string, std::unordered_map<std::string, TagList>> tags_;
    TagList vocab_ = {"@serialize", "@crypto", "@io", "@network", "@math",
                      "@collection", "@concurrency", "@test", "@ui", "@parse"};

    static std::filesystem::path defaultPath() {
        const char* home = std::getenv("USERPROFILE");
        if (!home) home = std::getenv("HOME");
        std::filesystem::path base = home ? home : ".";
        return base / ".whetstone" / "semantic_tags.json";
    }

    static std::string normalizeTag(const std::string& tag) {
        if (tag.empty()) return tag;
        if (tag[0] == '@') return tag;
        return "@" + tag;
    }

    static TagList dedupe(const TagList& tags) {
        std::unordered_set<std::string> seen;
        TagList out;
        for (const auto& t : tags) {
            if (t.empty()) continue;
            if (seen.insert(t).second) out.push_back(t);
        }
        return out;
    }

    static std::string lowerCopy(const std::string& input) {
        std::string out = input;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return out;
    }

    TagList autoTagsForLibrary(const std::string& library) const {
        std::string name = lowerCopy(library);
        if (name == "numpy" || name == "pandas" || name == "scipy") return {"@math"};
        if (name == "requests" || name == "httpx" || name == "axios") return {"@network"};
        if (name == "cryptography" || name == "openssl") return {"@crypto"};
        if (name == "json" || name == "yaml" || name == "toml" || name == "pickle" ||
            name == "serde") return {"@serialize"};
        if (name == "sqlite" || name == "sqlite3" || name == "sqlalchemy") return {"@io"};
        if (name == "pytest" || name == "unittest" || name == "jest") return {"@test"};
        if (name == "tkinter" || name == "react" || name == "qt") return {"@ui"};
        return {};
    }

    TagList inferTagsFromName(const std::string& name) const {
        std::string lower = lowerCopy(name);
        TagList out;
        if (lower.find("serialize") != std::string::npos || lower.find("json") != std::string::npos) {
            out.push_back("@serialize");
        }
        if (lower.find("encrypt") != std::string::npos || lower.find("decrypt") != std::string::npos ||
            lower.find("hash") != std::string::npos || lower.find("crypto") != std::string::npos) {
            out.push_back("@crypto");
        }
        if (lower.find("http") != std::string::npos || lower.find("request") != std::string::npos ||
            lower.find("socket") != std::string::npos || lower.find("net") != std::string::npos) {
            out.push_back("@network");
        }
        if (lower.find("read") != std::string::npos || lower.find("write") != std::string::npos ||
            lower.find("file") != std::string::npos || lower.find("stream") != std::string::npos) {
            out.push_back("@io");
        }
        if (lower.find("math") != std::string::npos || lower.find("calc") != std::string::npos ||
            lower.find("matrix") != std::string::npos) {
            out.push_back("@math");
        }
        if (lower.find("list") != std::string::npos || lower.find("map") != std::string::npos ||
            lower.find("dict") != std::string::npos || lower.find("set") != std::string::npos) {
            out.push_back("@collection");
        }
        if (lower.find("thread") != std::string::npos || lower.find("mutex") != std::string::npos ||
            lower.find("lock") != std::string::npos) {
            out.push_back("@concurrency");
        }
        if (lower.find("test") != std::string::npos) out.push_back("@test");
        if (lower.find("ui") != std::string::npos || lower.find("widget") != std::string::npos) {
            out.push_back("@ui");
        }
        if (lower.find("parse") != std::string::npos || lower.find("lexer") != std::string::npos) {
            out.push_back("@parse");
        }
        return out;
    }
};
