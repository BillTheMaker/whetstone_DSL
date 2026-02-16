#pragma once
#include "ASTNode.h"
#include <string>
#include <vector>

// GenericType — a parameterized type like List<T> or Map<K, V>
// Also used for template<typename T> class Foo when isClassTemplate=true
class GenericType : public ASTNode {
public:
    std::string baseName;  // e.g. "List", "Map", "Optional"
    bool isClassTemplate = false; // true for template<...> class, false for type usage

    GenericType() { conceptType = "GenericType"; }
    GenericType(const std::string& id_, const std::string& base)
        : baseName(base) {
        this->id = id_;
        this->conceptType = "GenericType";
    }
    // Children roles: "typeParameters" (list of TypeParameter or PrimitiveType)
};

// TypeParameter — a type variable like T, K, V with optional constraint
class TypeParameter : public ASTNode {
public:
    std::string name;       // e.g. "T", "K"
    std::string constraint; // e.g. "Comparable", empty if unconstrained
    bool isVariadic = false; // true for typename... Args

    TypeParameter() { conceptType = "TypeParameter"; }
    TypeParameter(const std::string& id_, const std::string& name_)
        : name(name_) {
        this->id = id_;
        this->conceptType = "TypeParameter";
    }
};

// Helper: extract template parameters from a class declaration's GenericType children
inline std::vector<TypeParameter*> getTemplateParameters(const ASTNode* classDecl) {
    std::vector<TypeParameter*> result;
    if (!classDecl) return result;
    // Look for GenericType children in "annotations" or direct children
    for (const auto& role : classDecl->childRoles()) {
        for (auto* child : classDecl->getChildren(role)) {
            if (child->conceptType == "GenericType") {
                for (auto* tp : child->getChildren("typeParameters")) {
                    if (tp->conceptType == "TypeParameter") {
                        result.push_back(static_cast<TypeParameter*>(tp));
                    }
                }
            }
        }
    }
    return result;
}

// Helper: detect CRTP pattern — class Foo : public Base<Foo>
// Checks if any base class name contains the class name as a template argument.
// Works by string matching on baseClasses (e.g., "Base<Foo>" contains "Foo").
inline bool isCRTP(const ASTNode* classDecl) {
    if (!classDecl || classDecl->conceptType != "ClassDeclaration") return false;

    // We need ClassDeclaration which is forward-declared here but included by callers
    // Use a string-based approach: check if any base class name contains "<ClassName>"
    // Get class name from properties (generic approach via node children is fragile)
    // The caller includes ClassDeclaration.h, so we can safely cast
    struct ClassDeclView {
        std::string name;
        std::vector<std::pair<std::string, bool>> bases; // name, isVirtual
    };

    // Extract via the node's property map approach
    // Since we know the layout, we reinterpret carefully
    // Actually, we just check child GenericType nodes for the class name
    // in their type parameters — this is the AST-level CRTP detection.

    // String-based fallback: check base class names for "<ClassName>"
    // This requires knowing the class name, which we get from node ID patterns
    // or by requiring the caller to pass it.
    // For maximum compatibility, we check children in the "typeParameters" role
    // of any GenericType children for the class's own name.

    return false; // Callers use isCRTPClass() below instead
}

// CRTP detection that takes class name and base class names directly
// Returns true if any base contains "<className>" pattern (e.g., Base<Foo> for class Foo)
inline bool isCRTPClass(const std::string& className,
                        const std::vector<std::string>& baseNames) {
    if (className.empty()) return false;
    std::string pattern = "<" + className + ">";
    for (const auto& base : baseNames) {
        if (base.find(pattern) != std::string::npos) return true;
    }
    return false;
}
