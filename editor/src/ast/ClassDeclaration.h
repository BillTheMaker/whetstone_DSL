#pragma once
#include "ASTNode.h"
#include "Function.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>

// BaseClass — represents a single base class with access specifier and virtual flag
struct BaseClass {
    std::string name;
    std::string accessSpecifier = "public"; // "public" | "protected" | "private"
    bool isVirtual = false;

    BaseClass() = default;
    BaseClass(const std::string& n, const std::string& access = "public", bool virt = false)
        : name(n), accessSpecifier(access), isVirtual(virt) {}
};

// ClassDeclaration — represents a class/struct definition
class ClassDeclaration : public ASTNode {
public:
    std::string name;
    std::string superClass;     // legacy single base class (deprecated, use baseClasses)
    bool isAbstract = false;
    std::vector<BaseClass> baseClasses;

    ClassDeclaration() { conceptType = "ClassDeclaration"; }
    ClassDeclaration(const std::string& id_, const std::string& name_)
        : name(name_) {
        this->id = id_;
        this->conceptType = "ClassDeclaration";
    }

    // Add a base class
    void addBase(const std::string& baseName, const std::string& access = "public",
                 bool isVirt = false) {
        baseClasses.emplace_back(baseName, access, isVirt);
        // Keep superClass in sync for backward compat
        if (baseClasses.size() == 1) superClass = baseName;
    }

    // Get all base classes; if baseClasses is empty but superClass is set, migrate
    std::vector<BaseClass> getBases() const {
        if (!baseClasses.empty()) return baseClasses;
        if (!superClass.empty()) {
            return { BaseClass(superClass, "public", false) };
        }
        return {};
    }

    // Detect diamond inheritance: two bases share a common ancestor
    // classMap maps class name → list of base class names
    static bool hasDiamondInheritance(
        const std::string& className,
        const std::map<std::string, std::vector<std::string>>& classMap)
    {
        auto it = classMap.find(className);
        if (it == classMap.end()) return false;
        const auto& directBases = it->second;
        if (directBases.size() < 2) return false;

        // Collect all ancestors of each direct base
        // Simple BFS: for each direct base, collect all transitive bases
        auto collectAncestors = [&](const std::string& base,
                                    std::vector<std::string>& ancestors) {
            std::vector<std::string> queue = { base };
            while (!queue.empty()) {
                std::string cur = queue.back();
                queue.pop_back();
                auto cit = classMap.find(cur);
                if (cit != classMap.end()) {
                    for (const auto& b : cit->second) {
                        ancestors.push_back(b);
                        queue.push_back(b);
                    }
                }
            }
        };

        // Check if any ancestor appears in more than one direct base's ancestry
        std::map<std::string, int> ancestorCount;
        for (const auto& base : directBases) {
            std::vector<std::string> ancestors;
            ancestors.push_back(base);
            collectAncestors(base, ancestors);
            // Use a set to avoid counting duplicates within one path
            std::vector<std::string> unique = ancestors;
            std::sort(unique.begin(), unique.end());
            unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
            for (const auto& a : unique) {
                ancestorCount[a]++;
            }
        }

        for (const auto& [ancestor, count] : ancestorCount) {
            if (count >= 2) return true;
        }
        return false;
    }

    // Children roles: "interfaces" (list of CustomType), "fields" (list of Variable),
    //                 "methods" (list of MethodDeclaration), "annotations"
};

// InterfaceDeclaration — represents an interface/trait/protocol
class InterfaceDeclaration : public ASTNode {
public:
    std::string name;

    InterfaceDeclaration() { conceptType = "InterfaceDeclaration"; }
    InterfaceDeclaration(const std::string& id_, const std::string& name_)
        : name(name_) {
        this->id = id_;
        this->conceptType = "InterfaceDeclaration";
    }
    // Children roles: "methods" (list of MethodDeclaration), "annotations"
};

// MethodDeclaration — extends Function with class-specific fields
class MethodDeclaration : public Function {
public:
    std::string className;
    bool isStatic = false;
    std::string visibility = "public";  // "public", "private", "protected"
    bool isOverride = false;
    bool isVirtual = false;

    MethodDeclaration() { conceptType = "MethodDeclaration"; }
    MethodDeclaration(const std::string& id_, const std::string& name_)
        : Function(id_, name_) {
        this->conceptType = "MethodDeclaration";
    }
    // Inherits children: "parameters", "body", "returnType", "annotations"
};
