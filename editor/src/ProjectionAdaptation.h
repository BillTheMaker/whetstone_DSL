#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include "ast/ASTNode.h"
#include "ast/Type.h"

namespace ProjectionAdaptation {

inline std::string adaptOwnerStrategy(const std::string& strategy,
                                      const std::string& targetLanguage) {
    if (targetLanguage == "rust") {
        if (strategy == "Manual") return "Box";
        if (strategy == "Borrowed") return "Borrowed";
        if (strategy == "Transferred") return "Moved";
    }
    return strategy;
}

inline void trimInPlace(std::string& text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.erase(text.begin());
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
}

inline bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

inline std::string stripPointers(const std::string& rawType) {
    std::string out = rawType;
    out.erase(std::remove(out.begin(), out.end(), '*'), out.end());
    trimInPlace(out);
    if (startsWith(out, "const ")) {
        out = out.substr(6);
        trimInPlace(out);
    }
    return out;
}

inline ASTNode* adaptPointerTypeForRust(ASTNode* typeNode) {
    if (!typeNode) return nullptr;
    if (typeNode->conceptType == "PrimitiveType") {
        auto* p = static_cast<PrimitiveType*>(typeNode);
        if (p->kind.find('*') == std::string::npos) return typeNode;
        std::string base = stripPointers(p->kind);
        if (!base.empty()) p->kind = "&" + base;
        return p;
    }
    if (typeNode->conceptType == "CustomType") {
        auto* c = static_cast<CustomType*>(typeNode);
        if (c->typeName.find('*') == std::string::npos) return typeNode;
        std::string base = stripPointers(c->typeName);
        if (!base.empty()) c->typeName = "&" + base;
        return c;
    }
    return typeNode;
}

inline ASTNode* adaptTypeForTarget(ASTNode* typeNode,
                                   const std::string& targetLanguage) {
    if (targetLanguage == "rust") {
        return adaptPointerTypeForRust(typeNode);
    }
    return typeNode;
}

}  // namespace ProjectionAdaptation
