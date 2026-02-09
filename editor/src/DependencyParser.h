#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <nlohmann/json.hpp>
#include "ast/Import.h"
#include "ast/Module.h"

struct DependencySpec {
    std::string name;
    std::string version;
    std::string source; // file or ecosystem hint
};

class DependencyParser {
public:
    static std::vector<DependencySpec> parseFile(const std::string& path) {
        std::filesystem::path p(path);
        std::string filename = p.filename().string();
        if (filename == "requirements.txt") return parseRequirements(path);
        if (filename == "package.json") return parsePackageJson(path);
        if (filename == "Cargo.toml") return parseCargoToml(path);
        if (filename == "go.mod") return parseGoMod(path);
        if (filename == "pom.xml") return parsePomXml(path);
        if (filename == "build.gradle") return parseGradle(path);
        if (filename == "vcpkg.json") return parseVcpkgJson(path);
        if (filename == "CMakeLists.txt") return parseCMakeLists(path);
        if (filename == "setup.py" || filename == "pyproject.toml") return parsePythonMeta(path);
        return {};
    }

    static void populateImports(Module* module, const std::vector<DependencySpec>& deps) {
        if (!module) return;
        for (const auto& dep : deps) {
            if (dep.name.empty()) continue;
            auto* imp = new Import("imp_" + dep.name, dep.name, "module");
            module->addChild("imports", imp);
        }
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos || end == std::string::npos) return "";
        return s.substr(start, end - start + 1);
    }

    static std::vector<DependencySpec> parseRequirements(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        while (std::getline(in, line)) {
            auto hash = line.find('#');
            if (hash != std::string::npos) line = line.substr(0, hash);
            line = trim(line);
            if (line.empty()) continue;
            DependencySpec dep;
            dep.source = "requirements.txt";
            std::string name = line;
            std::string version;
            size_t op = line.find("==");
            if (op == std::string::npos) op = line.find(">=");
            if (op == std::string::npos) op = line.find("<=");
            if (op != std::string::npos) {
                name = trim(line.substr(0, op));
                version = trim(line.substr(op + 2));
            }
            dep.name = name;
            dep.version = version;
            out.push_back(dep);
        }
        return out;
    }

    static std::vector<DependencySpec> parsePackageJson(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        nlohmann::json j;
        try { in >> j; } catch (...) { return out; }
        auto collect = [&](const std::string& key, const std::string& sourceTag) {
            if (!j.contains(key)) return;
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                DependencySpec dep;
                dep.name = it.key();
                dep.version = it.value().get<std::string>();
                dep.source = sourceTag;
                out.push_back(dep);
            }
        };
        collect("dependencies", "package.json:dependencies");
        collect("devDependencies", "package.json:devDependencies");
        return out;
    }

    static std::vector<DependencySpec> parseCargoToml(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        bool inDeps = false;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;
            if (line[0] == '[') {
                inDeps = (line == "[dependencies]");
                continue;
            }
            if (!inDeps) continue;
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string name = trim(line.substr(0, eq));
            std::string val = trim(line.substr(eq + 1));
            std::string version;
            if (!val.empty() && val[0] == '"') {
                auto end = val.find('"', 1);
                if (end != std::string::npos) version = val.substr(1, end - 1);
            } else {
                auto pos = val.find("version");
                if (pos != std::string::npos) {
                    auto q1 = val.find('"', pos);
                    auto q2 = q1 == std::string::npos ? std::string::npos : val.find('"', q1 + 1);
                    if (q1 != std::string::npos && q2 != std::string::npos) {
                        version = val.substr(q1 + 1, q2 - q1 - 1);
                    }
                }
            }
            DependencySpec dep;
            dep.name = name;
            dep.version = version;
            dep.source = "Cargo.toml";
            out.push_back(dep);
        }
        return out;
    }

    static std::vector<DependencySpec> parseGoMod(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        bool inRequire = false;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            if (line.rfind("require (", 0) == 0) { inRequire = true; continue; }
            if (inRequire && line == ")") { inRequire = false; continue; }
            if (line.rfind("require ", 0) == 0 && !inRequire) {
                line = trim(line.substr(8));
            } else if (!inRequire) {
                continue;
            }
            std::istringstream iss(line);
            std::string name, version;
            iss >> name >> version;
            if (name.empty()) continue;
            DependencySpec dep;
            dep.name = name;
            dep.version = version;
            dep.source = "go.mod";
            out.push_back(dep);
        }
        return out;
    }

    static std::vector<DependencySpec> parsePomXml(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string content((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
        size_t pos = 0;
        while ((pos = content.find("<dependency>", pos)) != std::string::npos) {
            size_t end = content.find("</dependency>", pos);
            if (end == std::string::npos) break;
            std::string block = content.substr(pos, end - pos);
            auto getTag = [&](const std::string& tag) {
                std::string open = "<" + tag + ">";
                std::string close = "</" + tag + ">";
                size_t a = block.find(open);
                size_t b = block.find(close);
                if (a == std::string::npos || b == std::string::npos) return std::string();
                return trim(block.substr(a + open.size(), b - a - open.size()));
            };
            std::string artifact = getTag("artifactId");
            std::string version = getTag("version");
            if (!artifact.empty()) {
                DependencySpec dep;
                dep.name = artifact;
                dep.version = version;
                dep.source = "pom.xml";
                out.push_back(dep);
            }
            pos = end + 1;
        }
        return out;
    }

    static std::vector<DependencySpec> parseGradle(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.find("implementation") == std::string::npos &&
                line.find("api") == std::string::npos) {
                continue;
            }
            auto q1 = line.find('"');
            auto q2 = q1 == std::string::npos ? std::string::npos : line.find('"', q1 + 1);
            if (q1 == std::string::npos || q2 == std::string::npos) continue;
            std::string depStr = line.substr(q1 + 1, q2 - q1 - 1);
            auto parts = split(depStr, ':');
            if (parts.size() >= 2) {
                DependencySpec dep;
                dep.name = parts[1];
                if (parts.size() >= 3) dep.version = parts[2];
                dep.source = "build.gradle";
                out.push_back(dep);
            }
        }
        return out;
    }

    static std::vector<DependencySpec> parseVcpkgJson(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        nlohmann::json j;
        try { in >> j; } catch (...) { return out; }
        if (!j.contains("dependencies")) return out;
        for (const auto& dep : j["dependencies"]) {
            DependencySpec spec;
            if (dep.is_string()) {
                spec.name = dep.get<std::string>();
            } else if (dep.is_object() && dep.contains("name")) {
                spec.name = dep["name"].get<std::string>();
                if (dep.contains("version")) spec.version = dep["version"].get<std::string>();
            }
            spec.source = "vcpkg.json";
            if (!spec.name.empty()) out.push_back(spec);
        }
        return out;
    }

    static std::vector<DependencySpec> parseCMakeLists(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        while (std::getline(in, line)) {
            auto pos = line.find("find_package(");
            if (pos == std::string::npos) continue;
            line = line.substr(pos + strlen("find_package("));
            auto end = line.find(')');
            if (end != std::string::npos) line = line.substr(0, end);
        std::istringstream iss(line);
        std::string name;
        iss >> name;
            if (!name.empty()) {
                DependencySpec dep;
                dep.name = name;
                dep.source = "CMakeLists.txt";
                out.push_back(dep);
            }
        }
        return out;
    }

    static std::vector<DependencySpec> parsePythonMeta(const std::string& path) {
        std::vector<DependencySpec> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string content((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
        auto pos = content.find("install_requires");
        if (pos == std::string::npos) return out;
        auto start = content.find('[', pos);
        auto end = content.find(']', start);
        if (start == std::string::npos || end == std::string::npos) return out;
        std::string list = content.substr(start + 1, end - start - 1);
        std::stringstream ss(list);
        std::string item;
        while (std::getline(ss, item, ',')) {
            item = trim(item);
            if (item.size() >= 2 && item.front() == '"' && item.back() == '"') {
                item = item.substr(1, item.size() - 2);
            } else if (item.size() >= 2 && item.front() == '\'' && item.back() == '\'') {
                item = item.substr(1, item.size() - 2);
            }
            if (!item.empty()) {
                DependencySpec dep;
                dep.name = item;
                dep.source = std::filesystem::path(path).filename().string();
                out.push_back(dep);
            }
        }
        return out;
    }

    static std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> parts;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            parts.push_back(item);
        }
        return parts;
    }
};
