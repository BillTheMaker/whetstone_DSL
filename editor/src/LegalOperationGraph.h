#pragma once
// Step 525: Legal Operation Graph

#include <string>
#include <vector>
#include <map>
#include <set>

struct LegalOpNode {
    std::string language;
    std::string nodeKind;
    std::set<std::string> operations;
};

struct LegalOpQueryResult {
    bool supported = false;
    std::vector<std::string> operations;
};

class LegalOperationGraph {
public:
    LegalOperationGraph() { initDefaults(); }

    void registerRule(const std::string& language,
                      const std::string& nodeKind,
                      const std::vector<std::string>& ops) {
        std::set<std::string> dedup(ops.begin(), ops.end());
        rules_[key(language, nodeKind)] = LegalOpNode{language, nodeKind, dedup};
    }

    LegalOpQueryResult allowedOps(const std::string& language,
                                  const std::string& nodeKind) const {
        auto it = rules_.find(key(language, nodeKind));
        if (it == rules_.end()) return {};
        LegalOpQueryResult r;
        r.supported = true;
        r.operations.assign(it->second.operations.begin(), it->second.operations.end());
        return r;
    }

    bool opAllowed(const std::string& language,
                   const std::string& nodeKind,
                   const std::string& op) const {
        auto q = allowedOps(language, nodeKind);
        if (!q.supported) return false;
        for (const auto& candidate : q.operations) {
            if (candidate == op) return true;
        }
        return false;
    }

private:
    std::map<std::string, LegalOpNode> rules_;

    static std::string key(const std::string& language,
                           const std::string& nodeKind) {
        return language + "::" + nodeKind;
    }

    void initDefaults() {
        // C/C++
        registerRule("cpp", "Function", {"rename", "update", "extract", "inline"});
        registerRule("cpp", "Variable", {"rename", "update", "delete"});
        registerRule("cpp", "Include", {"insert", "delete", "reorder"});
        registerRule("c", "Function", {"rename", "update", "extract"});

        // Python/TypeScript
        registerRule("python", "Function", {"rename", "update", "extract"});
        registerRule("python", "Class", {"rename", "update", "extract"});
        registerRule("typescript", "Function", {"rename", "update", "extract", "inline"});
        registerRule("typescript", "Import", {"insert", "delete", "reorder"});

        // Rust/Go
        registerRule("rust", "Function", {"rename", "update", "extract"});
        registerRule("rust", "Struct", {"rename", "update"});
        registerRule("go", "Function", {"rename", "update", "extract"});
        registerRule("go", "Import", {"insert", "delete", "reorder"});
    }
};
