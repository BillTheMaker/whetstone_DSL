#pragma once
// Step 689: Semantic core IR schema for cross-language transpilation.

#include <algorithm>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class IRNodeKind {
    Module,
    Function,
    Type,
    Effect,
    OwnershipRegion,
    ConcurrencyRegion,
    Unknown
};

struct IRNode {
    std::string id;
    IRNodeKind kind = IRNodeKind::Unknown;
    std::string name;
    std::string language;
    std::vector<std::string> intentTags;
    json annotations = json::object();
    json metadata = json::object();
};

struct IREdge {
    std::string fromId;
    std::string toId;
    std::string relation; // e.g. "calls", "owns", "spawns"
};

struct SemanticCoreIR {
    std::string moduleId;
    std::string moduleName;
    std::vector<IRNode> nodes;
    std::vector<IREdge> edges;
    json contracts = json::object(); // preconditions/postconditions/error contracts
    json extras = json::object();    // passthrough for forward compatibility
};

class SemanticCoreIRModel {
public:
    static std::string kindToString(IRNodeKind kind) {
        switch (kind) {
            case IRNodeKind::Module: return "module";
            case IRNodeKind::Function: return "function";
            case IRNodeKind::Type: return "type";
            case IRNodeKind::Effect: return "effect";
            case IRNodeKind::OwnershipRegion: return "ownership_region";
            case IRNodeKind::ConcurrencyRegion: return "concurrency_region";
            default: return "unknown";
        }
    }

    static IRNodeKind kindFromString(const std::string& s) {
        if (s == "module") return IRNodeKind::Module;
        if (s == "function") return IRNodeKind::Function;
        if (s == "type") return IRNodeKind::Type;
        if (s == "effect") return IRNodeKind::Effect;
        if (s == "ownership_region") return IRNodeKind::OwnershipRegion;
        if (s == "concurrency_region") return IRNodeKind::ConcurrencyRegion;
        return IRNodeKind::Unknown;
    }

    static json toJson(const SemanticCoreIR& ir) {
        json nodes = json::array();
        for (const auto& n : ir.nodes) {
            nodes.push_back({
                {"id", n.id},
                {"kind", kindToString(n.kind)},
                {"name", n.name},
                {"language", n.language},
                {"intentTags", n.intentTags},
                {"annotations", n.annotations},
                {"metadata", n.metadata}
            });
        }

        json edges = json::array();
        for (const auto& e : ir.edges) {
            edges.push_back({
                {"fromId", e.fromId},
                {"toId", e.toId},
                {"relation", e.relation}
            });
        }

        return {
            {"moduleId", ir.moduleId},
            {"moduleName", ir.moduleName},
            {"nodes", nodes},
            {"edges", edges},
            {"contracts", ir.contracts},
            {"extras", ir.extras}
        };
    }

    static bool fromJson(const json& j, SemanticCoreIR* out, std::string* error) {
        if (!out || !error) return false;
        *error = "";
        if (!j.is_object()) {
            *error = "ir_not_object";
            return false;
        }

        SemanticCoreIR ir;
        ir.moduleId = j.value("moduleId", "");
        ir.moduleName = j.value("moduleName", "");
        ir.contracts = j.value("contracts", json::object());
        ir.extras = j.value("extras", json::object());

        if (j.contains("nodes")) {
            if (!j["nodes"].is_array()) {
                *error = "nodes_not_array";
                return false;
            }
            for (const auto& n : j["nodes"]) {
                if (!n.is_object()) {
                    *error = "node_not_object";
                    return false;
                }
                IRNode node;
                node.id = n.value("id", "");
                node.kind = kindFromString(n.value("kind", "unknown"));
                node.name = n.value("name", "");
                node.language = n.value("language", "");
                node.intentTags = n.value("intentTags", std::vector<std::string>{});
                node.annotations = n.value("annotations", json::object());
                node.metadata = n.value("metadata", json::object());
                ir.nodes.push_back(std::move(node));
            }
        }

        if (j.contains("edges")) {
            if (!j["edges"].is_array()) {
                *error = "edges_not_array";
                return false;
            }
            for (const auto& e : j["edges"]) {
                if (!e.is_object()) {
                    *error = "edge_not_object";
                    return false;
                }
                IREdge edge;
                edge.fromId = e.value("fromId", "");
                edge.toId = e.value("toId", "");
                edge.relation = e.value("relation", "");
                ir.edges.push_back(std::move(edge));
            }
        }

        *out = std::move(ir);
        return true;
    }

    static bool validate(const SemanticCoreIR& ir, std::string* error) {
        if (error) *error = "";

        if (ir.moduleId.empty()) {
            if (error) *error = "module_id_missing";
            return false;
        }

        std::set<std::string> ids;
        for (const auto& n : ir.nodes) {
            if (n.id.empty()) {
                if (error) *error = "node_id_missing";
                return false;
            }
            if (!ids.insert(n.id).second) {
                if (error) *error = "node_id_duplicate";
                return false;
            }
        }

        for (const auto& e : ir.edges) {
            if (e.fromId.empty() || e.toId.empty() || e.relation.empty()) {
                if (error) *error = "edge_invalid";
                return false;
            }
            if (ids.find(e.fromId) == ids.end() || ids.find(e.toId) == ids.end()) {
                if (error) *error = "edge_endpoint_missing";
                return false;
            }
        }
        return true;
    }

    static const IRNode* findNode(const SemanticCoreIR& ir, const std::string& id) {
        for (const auto& n : ir.nodes) if (n.id == id) return &n;
        return nullptr;
    }

    static std::vector<IREdge> outgoing(const SemanticCoreIR& ir,
                                        const std::string& fromId,
                                        const std::string& relation = "") {
        std::vector<IREdge> out;
        for (const auto& e : ir.edges) {
            if (e.fromId != fromId) continue;
            if (!relation.empty() && e.relation != relation) continue;
            out.push_back(e);
        }
        return out;
    }
};
