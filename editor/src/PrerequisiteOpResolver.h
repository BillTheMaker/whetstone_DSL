#pragma once
// Step 679: Resolve prerequisiteOps to structured file/command metadata.

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

enum class OpKind { ReadFile, RunCommand, Unknown };

struct ResolvedOp {
    std::string raw;
    OpKind kind = OpKind::Unknown;
    std::string filePath;
    bool fileExists = false;
    int lineStart = 0;
    int lineEnd = 0;
};

class PrerequisiteOpResolver {
public:
    static ResolvedOp resolve(const std::string& op,
                              const std::string& workspaceRoot) {
        ResolvedOp out;
        out.raw = op;
        const std::string trimmed = trim(op);
        if (trimmed.empty()) return out;

        if (startsWith(trimmed, "read ")) {
            out.kind = OpKind::ReadFile;
            parseReadOp(trimmed, workspaceRoot, &out);
            return out;
        }

        if (startsWith(trimmed, "run ")) {
            out.kind = OpKind::RunCommand;
            return out;
        }

        return out;
    }

    static std::vector<ResolvedOp> resolveAll(const std::vector<std::string>& ops,
                                              const std::string& workspaceRoot) {
        std::vector<ResolvedOp> out;
        out.reserve(ops.size());
        for (const auto& op : ops) {
            out.push_back(resolve(op, workspaceRoot));
        }
        return out;
    }

private:
    static std::string trim(const std::string& s) {
        size_t a = 0;
        while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
        size_t b = s.size();
        while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
        return s.substr(a, b - a);
    }

    static bool startsWith(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    static std::string unquote(const std::string& s) {
        if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                              (s.front() == '\'' && s.back() == '\''))) {
            return s.substr(1, s.size() - 2);
        }
        return s;
    }

    static void parseReadOp(const std::string& op,
                            const std::string& workspaceRoot,
                            ResolvedOp* out) {
        if (!out) return;

        static const std::regex kLinesRe("\\slines\\s+([0-9]+)-([0-9]+)$");
        std::smatch m;
        std::string body = op.substr(5); // after "read "
        if (std::regex_search(body, m, kLinesRe)) {
            out->lineStart = std::stoi(m[1].str());
            out->lineEnd = std::stoi(m[2].str());
            body = body.substr(0, m.position());
        }

        body = trim(body);
        out->filePath = unquote(body);
        if (out->filePath.empty()) return;

        namespace fs = std::filesystem;
        fs::path p(out->filePath);
        if (p.is_relative()) p = fs::path(workspaceRoot) / p;
        std::error_code ec;
        out->fileExists = fs::exists(p, ec) && fs::is_regular_file(p, ec);
    }
};

