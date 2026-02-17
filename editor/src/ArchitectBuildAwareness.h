#pragma once

// Step 479: Dependency + Build System Awareness
// Adds build/dependency annotations from scaffold module dependencies.

#include "ArchitectSkeletonGenerator.h"

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

struct BuildDependencyAnnotation {
    std::string moduleName;
    std::string language;
    std::vector<std::string> moduleDependencies;
    std::vector<std::string> importHints;
    std::vector<std::string> packageDependencies;
    std::string manifestSnippet;
    int migrationOrder = 0; // SQL-only: 1-based order
    std::vector<std::string> notes;
};

struct BuildAwarenessReport {
    std::vector<BuildDependencyAnnotation> modules;
    std::vector<std::string> globalNotes;

    bool hasModule(const std::string& moduleName) const {
        for (const auto& m : modules) if (m.moduleName == moduleName) return true;
        return false;
    }

    BuildDependencyAnnotation getModule(const std::string& moduleName) const {
        for (const auto& m : modules) if (m.moduleName == moduleName) return m;
        return {};
    }
};

class ArchitectBuildAwareness {
public:
    static BuildAwarenessReport annotate(const SkeletonProjectSpec& skeleton) {
        BuildAwarenessReport out;
        const auto sqlOrder = computeSqlMigrationOrder(skeleton);

        for (const auto& m : skeleton.modules) {
            BuildDependencyAnnotation ann;
            ann.moduleName = m.moduleName;
            ann.language = lower(m.language);
            ann.moduleDependencies = m.dependencies;

            if (ann.language == "python") {
                annotatePython(m, ann);
            } else if (ann.language == "rust") {
                annotateRust(m, ann);
            } else if (ann.language == "typescript" || ann.language == "javascript") {
                annotateNode(m, ann);
            } else if (ann.language == "cpp" || ann.language == "c") {
                annotateCpp(m, ann);
            } else if (ann.language == "sql") {
                annotateSql(m, ann, sqlOrder);
            } else {
                annotateGeneric(m, ann);
            }

            out.modules.push_back(std::move(ann));
        }

        out.globalNotes.push_back("Build awareness report generated from module dependencies");
        out.globalNotes.push_back("Snippets are annotation-level hints, not executable build configs");
        return out;
    }

private:
    static std::string lower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static void annotatePython(const SkeletonModuleSpec& m, BuildDependencyAnnotation& ann) {
        for (const auto& dep : m.dependencies) {
            ann.importHints.push_back("from " + dep + " import *");
        }
        ann.packageDependencies.push_back("typing-extensions");
        if (m.moduleName.find("api") != std::string::npos) ann.packageDependencies.push_back("fastapi");
        if (m.moduleName.find("data") != std::string::npos) ann.packageDependencies.push_back("sqlalchemy");

        ann.manifestSnippet = "dependencies = [";
        for (size_t i = 0; i < ann.packageDependencies.size(); ++i) {
            if (i) ann.manifestSnippet += ", ";
            ann.manifestSnippet += "\"" + ann.packageDependencies[i] + "\"";
        }
        ann.manifestSnippet += "]";
        ann.notes.push_back("Python imports mapped from module graph");
    }

    static void annotateRust(const SkeletonModuleSpec& m, BuildDependencyAnnotation& ann) {
        ann.packageDependencies.push_back("serde = \"1\"");
        for (const auto& dep : m.dependencies) {
            ann.packageDependencies.push_back(dep + " = { path = \"../" + dep + "\" }");
            ann.importHints.push_back("use crate::" + dep + "::*;");
        }

        ann.manifestSnippet = "[dependencies]\n";
        for (const auto& p : ann.packageDependencies) ann.manifestSnippet += p + "\n";
        if (m.dependencies.empty()) ann.manifestSnippet += "# no local module dependencies\n";
        ann.notes.push_back("Cargo dependency hints generated from module dependencies");
    }

    static void annotateNode(const SkeletonModuleSpec& m, BuildDependencyAnnotation& ann) {
        ann.packageDependencies.push_back("typescript");
        if (m.moduleName.find("api") != std::string::npos) ann.packageDependencies.push_back("express");

        for (const auto& dep : m.dependencies) {
            ann.importHints.push_back("import * as " + dep + " from \"../" + dep + "/" + dep + "\";");
        }

        ann.manifestSnippet = "\"dependencies\": {";
        for (size_t i = 0; i < ann.packageDependencies.size(); ++i) {
            if (i) ann.manifestSnippet += ", ";
            ann.manifestSnippet += "\"" + ann.packageDependencies[i] + "\": \"*\"";
        }
        ann.manifestSnippet += "}";
        ann.notes.push_back("Node package and import hints generated");
    }

    static void annotateCpp(const SkeletonModuleSpec& m, BuildDependencyAnnotation& ann) {
        for (const auto& dep : m.dependencies) {
            ann.importHints.push_back("#include \"" + dep + ".h\"");
        }
        ann.packageDependencies.push_back("cmake-target:" + m.moduleName);

        std::string snippet = "add_library(" + m.moduleName + " INTERFACE)\n";
        if (!m.dependencies.empty()) {
            snippet += "target_link_libraries(" + m.moduleName + " INTERFACE";
            for (const auto& dep : m.dependencies) snippet += " " + dep;
            snippet += ")\n";
        }
        ann.manifestSnippet = snippet;
        ann.notes.push_back("CMake target/link hints generated from dependencies");
    }

    static void annotateSql(const SkeletonModuleSpec& m, BuildDependencyAnnotation& ann,
                            const std::map<std::string, int>& sqlOrder) {
        auto it = sqlOrder.find(m.moduleName);
        ann.migrationOrder = (it == sqlOrder.end()) ? 0 : it->second;
        ann.manifestSnippet = "-- migration_order: " + std::to_string(ann.migrationOrder);
        for (const auto& dep : m.dependencies) {
            ann.importHints.push_back("-- depends_on: " + dep);
        }
        ann.notes.push_back("SQL migration ordering derived from dependency graph");
    }

    static void annotateGeneric(const SkeletonModuleSpec& m, BuildDependencyAnnotation& ann) {
        ann.manifestSnippet = "# build-hints unavailable for language: " + lower(m.language);
        if (m.dependencies.empty()) ann.notes.push_back("No dependencies recorded");
        else ann.notes.push_back("Dependency links preserved for manual review");
    }

    static std::map<std::string, int> computeSqlMigrationOrder(const SkeletonProjectSpec& sk) {
        std::set<std::string> sql;
        for (const auto& m : sk.modules) {
            if (lower(m.language) == "sql") sql.insert(m.moduleName);
        }

        std::vector<std::string> ordered;
        std::set<std::string> temp, perm;
        bool hasCycle = false;

        std::function<void(const std::string&)> visit = [&](const std::string& node) {
            if (perm.count(node) || hasCycle) return;
            if (temp.count(node)) {
                hasCycle = true;
                return;
            }
            temp.insert(node);

            for (const auto& m : sk.modules) {
                if (m.moduleName != node) continue;
                for (const auto& dep : m.dependencies) {
                    if (sql.count(dep)) visit(dep);
                }
                break;
            }

            temp.erase(node);
            perm.insert(node);
            ordered.push_back(node);
        };

        for (const auto& n : sql) {
            if (!perm.count(n)) visit(n);
        }

        // Cycle fallback: deterministic lexical order.
        if (hasCycle) {
            ordered.assign(sql.begin(), sql.end());
            std::sort(ordered.begin(), ordered.end());
        }

        std::map<std::string, int> out;
        for (size_t i = 0; i < ordered.size(); ++i) out[ordered[i]] = (int)i + 1;
        return out;
    }
};
