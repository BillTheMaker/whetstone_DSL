#pragma once
// Steps 394-398: Advanced C++ AST node types
//
// Adds: const/constexpr/static flags on Variable/Function/MethodDeclaration,
// smart pointer recognition, auto type deduction, and cast expressions.

#include "ASTNode.h"
#include "Expression.h"
#include "Type.h"
#include <nlohmann/json.hpp>
#include <string>

// --- Extended Variable flags (Step 395) ---
// Rather than modify Variable.h (which is a tiny class used everywhere),
// we use a companion struct stored as annotation metadata.

struct CppQualifiers {
    bool isConst = false;
    bool isConstexpr = false;
    bool isStatic = false;
    bool isMutable = false;
    std::string smartPointerKind;  // "" | "unique_ptr" | "shared_ptr" | "weak_ptr"

    nlohmann::json toJson() const {
        nlohmann::json j;
        if (isConst) j["isConst"] = true;
        if (isConstexpr) j["isConstexpr"] = true;
        if (isStatic) j["isStatic"] = true;
        if (isMutable) j["isMutable"] = true;
        if (!smartPointerKind.empty()) j["smartPointerKind"] = smartPointerKind;
        return j;
    }

    static CppQualifiers fromJson(const nlohmann::json& j) {
        CppQualifiers q;
        if (j.contains("isConst")) q.isConst = j["isConst"].get<bool>();
        if (j.contains("isConstexpr")) q.isConstexpr = j["isConstexpr"].get<bool>();
        if (j.contains("isStatic")) q.isStatic = j["isStatic"].get<bool>();
        if (j.contains("isMutable")) q.isMutable = j["isMutable"].get<bool>();
        if (j.contains("smartPointerKind")) q.smartPointerKind = j["smartPointerKind"].get<std::string>();
        return q;
    }
};

// --- AutoType (Step 397) ---

class AutoType : public ASTNode {
public:
    std::string modifiers;  // "" | "*" | "&" | "const &" | "const *"

    AutoType() { conceptType = "AutoType"; }
    AutoType(const std::string& id_, const std::string& mods = "")
        : modifiers(mods) {
        this->id = id_;
        conceptType = "AutoType";
    }
};

// --- CastExpression (Step 398) ---

class CastExpression : public Expression {
public:
    std::string castKind;    // "static_cast" | "dynamic_cast" | "reinterpret_cast" | "const_cast"
    std::string targetType;  // The target type as string
    // children: "operand" (1) via setChild

    CastExpression() { conceptType = "CastExpression"; }
    CastExpression(const std::string& id_, const std::string& kind, const std::string& target = "")
        : castKind(kind), targetType(target) {
        this->id = id_;
        conceptType = "CastExpression";
    }
};

// --- Helper: detect smart pointer type from text ---

inline std::string detectSmartPointerKind(const std::string& typeText) {
    if (typeText.find("unique_ptr") != std::string::npos) return "unique_ptr";
    if (typeText.find("shared_ptr") != std::string::npos) return "shared_ptr";
    if (typeText.find("weak_ptr") != std::string::npos) return "weak_ptr";
    return "";
}

// --- Helper: extract inner type from smart pointer ---

inline std::string extractSmartPointerInner(const std::string& typeText) {
    auto start = typeText.find('<');
    auto end = typeText.rfind('>');
    if (start != std::string::npos && end != std::string::npos && end > start) {
        return typeText.substr(start + 1, end - start - 1);
    }
    return typeText;
}

// --- Helper: detect qualifiers from declaration text ---

inline CppQualifiers detectQualifiers(const std::string& text) {
    CppQualifiers q;
    if (text.find("constexpr ") != std::string::npos || text.find("constexpr\t") != std::string::npos)
        q.isConstexpr = true;
    if (text.find("const ") != std::string::npos || text.find("const\t") != std::string::npos)
        q.isConst = true;
    if (text.find("static ") != std::string::npos || text.find("static\t") != std::string::npos)
        q.isStatic = true;
    if (text.find("mutable ") != std::string::npos)
        q.isMutable = true;
    q.smartPointerKind = detectSmartPointerKind(text);
    return q;
}

// --- Helper: detect auto type from text ---

inline bool isAutoType(const std::string& typeText) {
    return typeText == "auto" || typeText.find("auto ") == 0 ||
           typeText.find("auto*") == 0 || typeText.find("auto&") == 0 ||
           typeText == "const auto" || typeText.find("const auto") == 0;
}

// --- Helper: detect const method (trailing const) ---

inline bool isConstMethod(const std::string& fullText) {
    // Look for ") const" or ") const {" or ") const;" pattern
    auto pos = fullText.rfind(')');
    if (pos != std::string::npos) {
        auto remainder = fullText.substr(pos + 1);
        return remainder.find("const") != std::string::npos &&
               remainder.find("const_cast") == std::string::npos;
    }
    return false;
}

// --- Helper: detect cast kind from text ---

inline std::string detectCastKind(const std::string& text) {
    if (text.find("dynamic_cast") != std::string::npos) return "dynamic_cast";
    if (text.find("reinterpret_cast") != std::string::npos) return "reinterpret_cast";
    if (text.find("const_cast") != std::string::npos) return "const_cast";
    if (text.find("static_cast") != std::string::npos) return "static_cast";
    return "static_cast"; // default
}

// --- Helper: map smart pointer to ownership annotation ---

inline std::string smartPointerOwnership(const std::string& kind) {
    if (kind == "unique_ptr") return "Unique";
    if (kind == "shared_ptr") return "Shared_ARC";
    if (kind == "weak_ptr") return "Weak";
    return "";
}

// --- Helper: risk level for cast ---

inline std::string castRiskLevel(const std::string& castKind) {
    if (castKind == "reinterpret_cast") return "high";
    if (castKind == "dynamic_cast") return "medium";
    if (castKind == "const_cast") return "medium";
    if (castKind == "static_cast") return "low";
    return "low";
}
