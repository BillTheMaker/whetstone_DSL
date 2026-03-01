#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace whetstone {

struct ASTFeatures {
    enum class RecursionShape { None, TailRecursive, TreeRecursive, FlatLoop };
    enum class ConcurrencyPrimitive { None, Channels, SharedMemory, Actors };
    enum class IOPattern { None, Blocking, Async, EventDriven };
    enum class TypeComplexity { None, SimpleGenerics, DependentTypes };

    float mutationRatio = 0.0f;
    RecursionShape recursionShape = RecursionShape::None;
    ConcurrencyPrimitive concurrencyPrimitive = ConcurrencyPrimitive::None;
    IOPattern ioPattern = IOPattern::None;
    TypeComplexity typeComplexity = TypeComplexity::None;
};

class ASTFeatureExtractor {
public:
    static ASTFeatures extract(const nlohmann::json& astNode) {
        std::vector<std::string> types;
        collectTypes(astNode, types, nullptr);

        ASTFeatures feat;
        feat.mutationRatio = computeMutationRatio(types);
        feat.recursionShape = detectRecursionShape(astNode, types);
        feat.concurrencyPrimitive = detectConcurrency(types);
        feat.ioPattern = detectIOPattern(types);
        feat.typeComplexity = detectTypeComplexity(types);
        return feat;
    }

private:
    static bool contains(const std::string& s, const std::string& sub) {
        return s.find(sub) != std::string::npos;
    }

    // Recursively collect all "type" strings from the JSON AST
    static void collectTypes(const nlohmann::json& node, std::vector<std::string>& out,
                             const std::string* parentType) {
        if (!node.is_object()) return;

        std::string nodeType;
        if (node.contains("type") && node["type"].is_string()) {
            nodeType = node["type"].get<std::string>();
            out.push_back(nodeType);
        }

        // Check children
        if (node.contains("children")) {
            const auto& children = node["children"];
            if (children.is_array()) {
                for (const auto& child : children) {
                    collectTypes(child, out, nodeType.empty() ? parentType : &nodeType);
                }
            } else if (children.is_object()) {
                // Structured children (e.g. {"methods": [...], "fields": [...]})
                for (auto& [key, val] : children.items()) {
                    if (val.is_array()) {
                        for (const auto& item : val) {
                            collectTypes(item, out, nodeType.empty() ? parentType : &nodeType);
                        }
                    } else if (val.is_object()) {
                        collectTypes(val, out, nodeType.empty() ? parentType : &nodeType);
                    }
                }
            }
        }
    }

    static float computeMutationRatio(const std::vector<std::string>& types) {
        if (types.empty()) return 0.0f;
        int mutations = 0;
        for (const auto& t : types) {
            if (contains(t, "assign") || contains(t, "write") ||
                contains(t, "store") || contains(t, "set_")) {
                ++mutations;
            }
        }
        return static_cast<float>(mutations) / static_cast<float>(types.size());
    }

    static ASTFeatures::RecursionShape detectRecursionShape(
        const nlohmann::json& root, const std::vector<std::string>& types)
    {
        // FlatLoop: any loop node
        for (const auto& t : types) {
            if (contains(t, "for_") || contains(t, "while") || contains(t, "loop")) {
                return ASTFeatures::RecursionShape::FlatLoop;
            }
        }

        // TailRecursive: call_expr is a direct child of return_stmt anywhere in tree
        if (hasTailCall(root)) {
            return ASTFeatures::RecursionShape::TailRecursive;
        }

        // TreeRecursive: 2+ call_expr nodes without any loop
        int callCount = 0;
        for (const auto& t : types) {
            if (contains(t, "call_")) ++callCount;
        }
        if (callCount >= 2) {
            return ASTFeatures::RecursionShape::TreeRecursive;
        }

        return ASTFeatures::RecursionShape::None;
    }

    // Returns true if any return_stmt node has a call_expr direct child
    static bool hasTailCall(const nlohmann::json& node) {
        if (!node.is_object()) return false;

        std::string nodeType;
        if (node.contains("type") && node["type"].is_string()) {
            nodeType = node["type"].get<std::string>();
        }

        if (nodeType == "return_stmt" && node.contains("children")) {
            const auto& ch = node["children"];
            if (ch.is_array()) {
                for (const auto& child : ch) {
                    if (child.is_object() && child.contains("type")) {
                        std::string ct = child["type"].get<std::string>();
                        if (contains(ct, "call_")) return true;
                    }
                }
            }
        }

        // Recurse
        if (node.contains("children")) {
            const auto& children = node["children"];
            if (children.is_array()) {
                for (const auto& child : children) {
                    if (hasTailCall(child)) return true;
                }
            } else if (children.is_object()) {
                for (auto& [key, val] : children.items()) {
                    if (val.is_array()) {
                        for (const auto& item : val) {
                            if (hasTailCall(item)) return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    static ASTFeatures::ConcurrencyPrimitive detectConcurrency(
        const std::vector<std::string>& types)
    {
        for (const auto& t : types) {
            if (contains(t, "actor") || contains(t, "spawn") ||
                contains(t, "gen_server") || (contains(t, "receive") && !contains(t, "chan"))) {
                return ASTFeatures::ConcurrencyPrimitive::Actors;
            }
        }
        for (const auto& t : types) {
            if (contains(t, "mutex") || contains(t, "lock") ||
                contains(t, "atomic") || contains(t, "shared_mem")) {
                return ASTFeatures::ConcurrencyPrimitive::SharedMemory;
            }
        }
        for (const auto& t : types) {
            if (contains(t, "chan") || contains(t, "channel") ||
                contains(t, "_send") || contains(t, "_recv")) {
                return ASTFeatures::ConcurrencyPrimitive::Channels;
            }
        }
        return ASTFeatures::ConcurrencyPrimitive::None;
    }

    static ASTFeatures::IOPattern detectIOPattern(const std::vector<std::string>& types) {
        for (const auto& t : types) {
            if (contains(t, "async") || contains(t, "await") ||
                contains(t, "future") || contains(t, "promise")) {
                return ASTFeatures::IOPattern::Async;
            }
        }
        for (const auto& t : types) {
            if (contains(t, "event") || contains(t, "handler") ||
                contains(t, "emit") || contains(t, "listen")) {
                return ASTFeatures::IOPattern::EventDriven;
            }
        }
        for (const auto& t : types) {
            if (contains(t, "read_") || contains(t, "write_") ||
                contains(t, "open_") || contains(t, "close_") || contains(t, "socket")) {
                return ASTFeatures::IOPattern::Blocking;
            }
        }
        return ASTFeatures::IOPattern::None;
    }

    static ASTFeatures::TypeComplexity detectTypeComplexity(
        const std::vector<std::string>& types)
    {
        for (const auto& t : types) {
            if (contains(t, "dependent") || contains(t, "refinement") ||
                contains(t, "pi_type")) {
                return ASTFeatures::TypeComplexity::DependentTypes;
            }
        }
        for (const auto& t : types) {
            if (contains(t, "template") || contains(t, "generic") ||
                contains(t, "type_param")) {
                return ASTFeatures::TypeComplexity::SimpleGenerics;
            }
        }
        return ASTFeatures::TypeComplexity::None;
    }
};

} // namespace whetstone
