#pragma once

// Step 478: Scaffold File Generation
// Materializes an approved skeleton into concrete files and file operations.

#include "ArchitectSkeletonGenerator.h"
#include "FileOperations.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

struct ScaffoldFileSpec {
    std::string relativePath;
    std::string language;
    std::string content;
};

struct ScaffoldOperation {
    std::string method;   // fileCreate | fileWrite
    std::string path;     // workspace-relative path
    std::string language; // for fileCreate
    std::string content;  // for fileWrite
};

struct ScaffoldPlan {
    std::string workspaceRoot;
    std::string projectName;
    std::vector<std::string> directories;
    std::vector<ScaffoldFileSpec> files;
    std::vector<ScaffoldOperation> operations;
    std::vector<std::string> notes;

    bool hasFile(const std::string& relPath) const {
        for (const auto& f : files) {
            if (f.relativePath == relPath) return true;
        }
        return false;
    }
};

struct ScaffoldInput {
    std::string workspaceRoot; // target workspace root
    std::string projectName;
    SkeletonProjectSpec skeleton;
    bool includeWhetstoneState = true;
};

class ArchitectScaffoldGenerator {
public:
    static ScaffoldPlan buildPlan(const ScaffoldInput& in) {
        ScaffoldPlan out;
        out.workspaceRoot = in.workspaceRoot.empty() ? "." : in.workspaceRoot;
        out.projectName = sanitizeSegment(in.projectName.empty() ? "project" : in.projectName);
        const std::string base = out.projectName;

        std::vector<std::string> languages;
        for (const auto& m : in.skeleton.modules) {
            languages.push_back(lower(m.language));
            const std::string rel = base + "/" + modulePath(m);
            out.files.push_back({rel, lower(m.language), renderModuleFile(m)});
        }

        if (containsAny(languages, {"typescript", "javascript"})) {
            out.files.push_back({base + "/package.json", "json", renderPackageJson(out.projectName)});
        }
        if (containsAny(languages, {"python"})) {
            out.files.push_back({base + "/pyproject.toml", "toml", renderPyproject(out.projectName)});
        }
        if (containsAny(languages, {"rust"})) {
            out.files.push_back({base + "/Cargo.toml", "toml", renderCargo(out.projectName)});
        }
        if (containsAny(languages, {"cpp", "c"})) {
            out.files.push_back({base + "/CMakeLists.txt", "cmake", renderCmake(out.projectName)});
        }

        out.files.push_back({base + "/.gitignore", "text", renderGitignore(languages)});

        if (in.includeWhetstoneState) {
            out.files.push_back({base + "/.whetstone/workflow_state.json",
                                 "json", renderWorkflowState(in.skeleton)});
            for (const auto& m : in.skeleton.modules) {
                const std::string sidecar =
                    base + "/.whetstone/sidecars/" + sanitizeSegment(m.moduleName) + ".ast.json";
                out.files.push_back({sidecar, "json", renderSidecar(m)});
            }
        }

        collectDirectories(out);
        buildOperations(out);
        out.notes.push_back("Scaffold plan generated from approved skeleton");
        out.notes.push_back("Operations are compatible with fileCreate/fileWrite MCP semantics");
        return out;
    }

    static bool applyPlan(const ScaffoldPlan& plan, std::string* error = nullptr) {
        for (const auto& op : plan.operations) {
            auto [okPath, resolved] = fileOpsResolvePath(plan.workspaceRoot, op.path);
            if (!okPath) {
                if (error) *error = resolved;
                return false;
            }

            if (op.method == "fileCreate") {
                if (std::filesystem::exists(resolved)) continue;
                auto [ok, msg] = fileOpsCreate(resolved, op.language, "empty");
                if (!ok) {
                    if (error) *error = msg;
                    return false;
                }
                continue;
            }

            if (op.method == "fileWrite") {
                auto [ok, msg, bytes] = fileOpsWrite(resolved, op.content);
                (void)bytes;
                if (!ok) {
                    if (error) *error = msg;
                    return false;
                }
                continue;
            }

            if (error) *error = "Unknown operation method: " + op.method;
            return false;
        }
        return true;
    }

private:
    static std::string lower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static std::string sanitizeSegment(const std::string& s) {
        std::string out;
        for (char c : s) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (std::isalnum(uc) || c == '_' || c == '-') out.push_back(c);
            else if (std::isspace(uc)) out.push_back('_');
        }
        if (out.empty()) return "project";
        return out;
    }

    static bool containsAny(const std::vector<std::string>& values,
                            std::initializer_list<const char*> expected) {
        for (const auto& e : expected) {
            for (const auto& v : values) {
                if (v == e) return true;
            }
        }
        return false;
    }

    static std::string modulePath(const SkeletonModuleSpec& m) {
        const std::string mod = sanitizeSegment(m.moduleName);
        const std::string lang = lower(m.language);
        if (lang == "typescript") return "src/" + mod + "/" + mod + ".ts";
        if (lang == "javascript") return "src/" + mod + "/" + mod + ".js";
        if (lang == "python") return "src/" + mod + ".py";
        if (lang == "rust") return "src/" + mod + ".rs";
        if (lang == "cpp" || lang == "c") return "src/" + mod + ".h";
        if (lang == "go") return "internal/" + mod + "/" + mod + ".go";
        if (lang == "sql") return "db/" + mod + ".sql";
        if (lang == "yaml" || lang == "yml") return "deploy/" + mod + ".yaml";
        if (lang == "markdown" || lang == "md") return "docs/" + mod + ".md";
        return "src/" + mod + ".txt";
    }

    static std::string annotationPrefix(const std::string& lang) {
        if (lang == "sql") return "-- ";
        if (lang == "markdown" || lang == "md") return "<!-- ";
        return "// ";
    }

    static std::string annotationSuffix(const std::string& lang) {
        if (lang == "markdown" || lang == "md") return " -->";
        return "";
    }

    static std::string toIdentifier(const std::string& name) {
        std::string out;
        for (char c : name) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (std::isalnum(uc) || c == '_') out.push_back(c);
            else out.push_back('_');
        }
        if (out.empty()) return "generated_fn";
        if (std::isdigit(static_cast<unsigned char>(out[0]))) out = "fn_" + out;
        return out;
    }

    static std::string renderFunction(const SkeletonFunctionSpec& fn, const std::string& lang) {
        std::ostringstream o;
        const std::string prefix = annotationPrefix(lang);
        const std::string suffix = annotationSuffix(lang);
        for (const auto& a : fn.annotations) {
            o << prefix << a.key << ": " << a.value << suffix << "\n";
        }

        const std::string id = toIdentifier(fn.name);
        if (lang == "python") {
            o << "def " << id << "():\n    pass\n\n";
        } else if (lang == "typescript") {
            o << "export function " << id << "(): void {\n}\n\n";
        } else if (lang == "javascript") {
            o << "export function " << id << "() {\n}\n\n";
        } else if (lang == "rust") {
            o << "pub fn " << id << "() {\n}\n\n";
        } else if (lang == "go") {
            o << "func " << id << "() {\n}\n\n";
        } else if (lang == "cpp" || lang == "c") {
            o << "inline void " << id << "() {\n}\n\n";
        } else if (lang == "sql") {
            o << "-- STUB: " << id << "\n\n";
        } else if (lang == "yaml" || lang == "yml") {
            o << id << ": stub\n\n";
        } else if (lang == "markdown" || lang == "md") {
            o << "### " << id << "\n\n";
        } else {
            o << id << "\n\n";
        }
        return o.str();
    }

    static std::string renderModuleFile(const SkeletonModuleSpec& m) {
        std::ostringstream o;
        const std::string lang = lower(m.language);
        if (lang == "python") o << "# Auto-generated scaffold module: " << m.moduleName << "\n\n";
        else if (lang == "sql") o << "-- Auto-generated scaffold module: " << m.moduleName << "\n\n";
        else if (lang == "markdown" || lang == "md")
            o << "# Auto-generated scaffold module: " << m.moduleName << "\n\n";
        else o << "// Auto-generated scaffold module: " << m.moduleName << "\n\n";

        if (!m.dependencies.empty()) {
            if (lang == "python") o << "# dependencies: ";
            else if (lang == "sql") o << "-- dependencies: ";
            else if (lang == "markdown" || lang == "md") o << "Dependencies: ";
            else o << "// dependencies: ";
            for (size_t i = 0; i < m.dependencies.size(); ++i) {
                if (i) o << ", ";
                o << m.dependencies[i];
            }
            o << "\n\n";
        }

        for (const auto& fn : m.functions) o << renderFunction(fn, lang);
        if (m.functions.empty()) o << renderFunction({"execute", "void execute()", {}}, lang);
        return o.str();
    }

    static std::string renderPackageJson(const std::string& projectName) {
        std::ostringstream o;
        o << "{\n"
          << "  \"name\": \"" << projectName << "\",\n"
          << "  \"version\": \"0.1.0\",\n"
          << "  \"private\": true,\n"
          << "  \"scripts\": {\n"
          << "    \"build\": \"echo build\",\n"
          << "    \"test\": \"echo test\"\n"
          << "  }\n"
          << "}\n";
        return o.str();
    }

    static std::string renderPyproject(const std::string& projectName) {
        std::ostringstream o;
        o << "[project]\n"
          << "name = \"" << projectName << "\"\n"
          << "version = \"0.1.0\"\n"
          << "description = \"Generated by ArchitectScaffoldGenerator\"\n";
        return o.str();
    }

    static std::string renderCargo(const std::string& projectName) {
        std::ostringstream o;
        o << "[package]\n"
          << "name = \"" << projectName << "\"\n"
          << "version = \"0.1.0\"\n"
          << "edition = \"2021\"\n";
        return o.str();
    }

    static std::string renderCmake(const std::string& projectName) {
        std::ostringstream o;
        o << "cmake_minimum_required(VERSION 3.20)\n"
          << "project(" << projectName << " LANGUAGES CXX)\n\n"
          << "add_library(" << projectName << "_scaffold INTERFACE)\n";
        return o.str();
    }

    static std::string renderGitignore(const std::vector<std::string>& languages) {
        std::ostringstream o;
        o << ".DS_Store\n.idea/\n.vscode/\n";
        if (containsAny(languages, {"typescript", "javascript"})) o << "node_modules/\ndist/\n";
        if (containsAny(languages, {"python"})) o << "__pycache__/\n*.pyc\n.venv/\n";
        if (containsAny(languages, {"rust"})) o << "target/\n";
        if (containsAny(languages, {"cpp", "c"})) o << "build/\n*.o\n";
        o << ".whetstone/\n";
        return o.str();
    }

    static std::string renderWorkflowState(const SkeletonProjectSpec& sk) {
        std::ostringstream o;
        o << "{\n"
          << "  \"status\": \"planned\",\n"
          << "  \"moduleCount\": " << sk.modules.size() << ",\n"
          << "  \"modules\": [";
        for (size_t i = 0; i < sk.modules.size(); ++i) {
            if (i) o << ", ";
            o << "\"" << sanitizeSegment(sk.modules[i].moduleName) << "\"";
        }
        o << "]\n}\n";
        return o.str();
    }

    static std::string renderSidecar(const SkeletonModuleSpec& mod) {
        std::ostringstream o;
        o << "{\n"
          << "  \"module\": \"" << sanitizeSegment(mod.moduleName) << "\",\n"
          << "  \"language\": \"" << lower(mod.language) << "\",\n"
          << "  \"functions\": [";
        for (size_t i = 0; i < mod.functions.size(); ++i) {
            if (i) o << ", ";
            o << "{ \"name\": \"" << toIdentifier(mod.functions[i].name) << "\" }";
        }
        o << "]\n}\n";
        return o.str();
    }

    static void collectDirectories(ScaffoldPlan& plan) {
        std::vector<std::string> uniqueDirs;
        for (const auto& f : plan.files) {
            std::filesystem::path p(f.relativePath);
            auto dir = p.parent_path().string();
            if (dir.empty()) continue;
            if (std::find(uniqueDirs.begin(), uniqueDirs.end(), dir) == uniqueDirs.end()) {
                uniqueDirs.push_back(dir);
            }
        }
        plan.directories = std::move(uniqueDirs);
    }

    static void buildOperations(ScaffoldPlan& plan) {
        plan.operations.clear();
        for (const auto& f : plan.files) {
            plan.operations.push_back({"fileCreate", f.relativePath, f.language, ""});
            plan.operations.push_back({"fileWrite", f.relativePath, "", f.content});
        }
    }
};
