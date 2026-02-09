#pragma once
// Step 96: LSP server configuration settings

#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include "ast/Schema.h"
#include "ast/ASTNode.h"

struct LSPServerConfig {
    std::string language;
    std::string server;
    std::string path;
    std::vector<std::string> args;
    std::string argsLine;
    bool enabled = true;
};

class SettingsManager {
public:
    SettingsManager() { loadDefaults(); }

    const std::vector<LSPServerConfig>& getLSPServers() const { return lspServers_; }
    std::vector<LSPServerConfig>& getLSPServersMutable() { return lspServers_; }
    const std::string& getEmacsConfigPath() const { return emacsConfigPath_; }
    void setEmacsConfigPath(const std::string& path) { emacsConfigPath_ = path; }
    std::string& getEmacsConfigPathMutable() { return emacsConfigPath_; }

    LSPServerConfig* getServer(const std::string& language) {
        for (auto& s : lspServers_) {
            if (s.language == language) return &s;
        }
        return nullptr;
    }

    void setServerPath(const std::string& language, const std::string& path) {
        auto* cfg = getServer(language);
        if (cfg) cfg->path = path;
    }

    void syncArgs(LSPServerConfig& cfg) {
        cfg.args = splitArgs(cfg.argsLine);
    }

    void autoDetect() {
        for (auto& s : lspServers_) {
            if (!s.path.empty()) continue;
            std::string found;
            if (findOnPath(s.server, found)) {
                s.path = found;
            }
        }
    }

    bool validateSchema(const ASTNode* root, std::vector<std::string>& errors) const {
        errors.clear();
        if (!root) {
            errors.push_back("Null AST root");
            return false;
        }
        ASTSchema schema;
        validateNode(root, schema, errors);
        return errors.empty();
    }

private:
    std::vector<LSPServerConfig> lspServers_;
    std::string emacsConfigPath_;

    void loadDefaults() {
        lspServers_.push_back(makeConfig("python", "pylsp", {}));
        lspServers_.push_back(makeConfig("cpp", "clangd", {}));
        lspServers_.push_back(makeConfig("rust", "rust-analyzer", {}));
        lspServers_.push_back(makeConfig("go", "gopls", {}));
        lspServers_.push_back(makeConfig("javascript", "typescript-language-server", {"--stdio"}));
        lspServers_.push_back(makeConfig("typescript", "typescript-language-server", {"--stdio"}));
        lspServers_.push_back(makeConfig("java", "jdtls", {}));

        const char* home = std::getenv("USERPROFILE");
        if (!home) home = std::getenv("HOME");
        if (home) {
            emacsConfigPath_ = (std::filesystem::path(home) / ".emacs.d").string();
        } else {
            emacsConfigPath_ = ".emacs.d";
        }
    }

    static LSPServerConfig makeConfig(const std::string& lang,
                                      const std::string& server,
                                      const std::vector<std::string>& args) {
        LSPServerConfig cfg;
        cfg.language = lang;
        cfg.server = server;
        cfg.args = args;
        cfg.argsLine = joinArgs(args);
        cfg.enabled = true;
        return cfg;
    }

    static std::string joinArgs(const std::vector<std::string>& args) {
        std::string out;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) out += " ";
            out += args[i];
        }
        return out;
    }

    static std::vector<std::string> splitArgs(const std::string& line) {
        std::vector<std::string> out;
        std::string current;
        for (char c : line) {
            if (c == ' ') {
                if (!current.empty()) {
                    out.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) out.push_back(current);
        return out;
    }

    static bool findOnPath(const std::string& exe, std::string& outPath) {
        const char* pathEnv = std::getenv("PATH");
        if (!pathEnv) return false;
        std::string pathVar(pathEnv);
        size_t start = 0;
        while (start <= pathVar.size()) {
            size_t end = pathVar.find(';', start);
            if (end == std::string::npos) end = pathVar.size();
            std::string dir = pathVar.substr(start, end - start);
            if (!dir.empty()) {
                std::filesystem::path p = std::filesystem::path(dir) / exe;
                if (std::filesystem::exists(p)) {
                    outPath = p.string();
                    return true;
                }
                std::filesystem::path pExe = p;
                pExe += ".exe";
                if (std::filesystem::exists(pExe)) {
                    outPath = pExe.string();
                    return true;
                }
            }
            if (end == pathVar.size()) break;
            start = end + 1;
        }
        return false;
    }

    static void validateNode(const ASTNode* node, const ASTSchema& schema, std::vector<std::string>& errors) {
        if (!node) return;
        for (const auto& role : node->childRoles()) {
            const auto& kids = node->getChildren(role);
            if (schema.isSingleValued(node->conceptType, role) && kids.size() > 1) {
                errors.push_back("Role '" + role + "' on " + node->conceptType + " is single-valued");
            }
            for (const auto* child : kids) {
                if (!schema.isLegalChild(node->conceptType, role, child->conceptType)) {
                    errors.push_back("Illegal child " + child->conceptType + " under " + node->conceptType + " role '" + role + "'");
                }
                validateNode(child, schema, errors);
            }
        }
    }
};
