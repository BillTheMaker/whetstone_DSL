#pragma once
// Step 1546: Semantic hash primitives and deterministic hash computation.

#include "ast/ASTNode.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

inline std::string semanticIrNodeKindForConcept(const std::string& conceptType) {
    if (conceptType == "Module") return "Module";
    if (conceptType == "Function" || conceptType == "MethodDeclaration" ||
        conceptType == "AsyncFunction") return "Function";
    if (conceptType == "ClassDeclaration" || conceptType == "InterfaceDeclaration" ||
        conceptType == "TypeAlias" || conceptType == "GenericType" ||
        conceptType == "TypeParameter" || conceptType == "PrimitiveType" ||
        conceptType == "CustomType") return "Type";
    if (conceptType == "HostCall" || conceptType == "ScheduleTask" ||
        conceptType == "ModuleLoad") return "Effect";
    return "Node";
}

inline void collectSemanticPrimitivePaths(const json& value,
                                          const std::string& prefix,
                                          std::set<std::string>& out) {
    if (value.is_object()) {
        for (auto it = value.begin(); it != value.end(); ++it) {
            const std::string key = prefix.empty() ? it.key() : (prefix + "." + it.key());
            collectSemanticPrimitivePaths(it.value(), key, out);
        }
        return;
    }
    if (value.is_array()) {
        if (!prefix.empty()) out.insert(prefix + "[]");
        return;
    }
    if (!prefix.empty()) out.insert(prefix);
}

inline std::vector<std::string> semanticPrimitivePaths(const json& semanticSummary) {
    std::set<std::string> unique;
    collectSemanticPrimitivePaths(semanticSummary, "", unique);
    return std::vector<std::string>(unique.begin(), unique.end());
}

inline uint64_t fnv1a64(const std::string& input) {
    constexpr uint64_t kOffset = 14695981039346656037ull;
    constexpr uint64_t kPrime = 1099511628211ull;
    uint64_t hash = kOffset;
    for (unsigned char c : input) {
        hash ^= static_cast<uint64_t>(c);
        hash *= kPrime;
    }
    return hash;
}

inline std::string formatSemanticHashV1(uint64_t value) {
    std::ostringstream oss;
    oss << "H1:" << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << value;
    return oss.str();
}

inline json semanticHashPreimage(const ASTNode* node, const json& semanticSummary) {
    json preimage = json::object();
    preimage["concept"] = node ? node->conceptType : "Unknown";
    preimage["irNodeKind"] = node ? semanticIrNodeKindForConcept(node->conceptType) : "Node";
    preimage["semantic"] = semanticSummary.is_object() ? semanticSummary : json::object();
    preimage["primitives"] = semanticPrimitivePaths(preimage["semantic"]);
    return preimage;
}

inline std::string computeSemanticHashV1(const ASTNode* node, const json& semanticSummary) {
    const json preimage = semanticHashPreimage(node, semanticSummary);
    return formatSemanticHashV1(fnv1a64(preimage.dump()));
}

inline json semanticHashPreimageV2(const ASTNode* node, const json& semanticSummary) {
    json preimage = semanticHashPreimage(node, semanticSummary);
    preimage["hashSchemaVersion"] = 2;
    preimage["hashProfile"] = "semantic-contract-v2";
    return preimage;
}

inline std::string computeSemanticHashV2(const ASTNode* node, const json& semanticSummary) {
    const json preimage = semanticHashPreimageV2(node, semanticSummary);
    return "H2:" + formatSemanticHashV1(fnv1a64(preimage.dump())).substr(3);
}

struct SemanticHashDigests {
    std::string h1;
    std::string h2;
};

inline SemanticHashDigests computeSemanticHashDigests(const ASTNode* node, const json& semanticSummary) {
    SemanticHashDigests out;
    out.h1 = computeSemanticHashV1(node, semanticSummary);
    out.h2 = computeSemanticHashV2(node, semanticSummary);
    return out;
}
