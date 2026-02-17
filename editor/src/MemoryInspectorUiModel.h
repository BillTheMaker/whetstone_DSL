#pragma once
// Step 565: Memory Inspector UI Model

#include "MemorySnapshotModel.h"

#include <cstdint>
#include <string>
#include <vector>

enum class MemoryValueKind {
    Integer,
    Floating,
    Boolean,
    String,
    Pointer,
    Container,
    Struct
};

struct MemoryValueNode {
    std::string nodeId;
    std::string label;
    MemoryValueKind kind = MemoryValueKind::Integer;
    std::string valueText;
    bool expanded = false;
    std::vector<MemoryValueNode> children;
};

struct MemoryInspectorRow {
    std::string nodeId;
    std::string label;
    std::string typeName;
    std::string valueText;
    int depth = 0;
    bool expandable = false;
    bool expanded = false;
};

class MemoryInspectorUiModel {
public:
    static bool createLeaf(const std::string& nodeId,
                           const std::string& label,
                           MemoryValueKind kind,
                           const std::string& valueText,
                           MemoryValueNode* outNode,
                           std::string* error) {
        if (!outNode || !error) return false;
        error->clear();
        if (nodeId.empty() || label.empty()) {
            *error = "node_id_or_label_missing";
            return false;
        }
        if (isCompositeKind(kind)) {
            *error = "leaf_kind_must_be_primitive";
            return false;
        }

        MemoryValueNode node;
        node.nodeId = nodeId;
        node.label = label;
        node.kind = kind;
        node.valueText = valueText;
        *outNode = node;
        return true;
    }

    static bool createComposite(const std::string& nodeId,
                                const std::string& label,
                                MemoryValueKind kind,
                                MemoryValueNode* outNode,
                                std::string* error) {
        if (!outNode || !error) return false;
        error->clear();
        if (nodeId.empty() || label.empty()) {
            *error = "node_id_or_label_missing";
            return false;
        }
        if (!isCompositeKind(kind)) {
            *error = "composite_kind_required";
            return false;
        }

        MemoryValueNode node;
        node.nodeId = nodeId;
        node.label = label;
        node.kind = kind;
        node.expanded = true;
        *outNode = node;
        return true;
    }

    static bool appendChild(MemoryValueNode* parent,
                            const MemoryValueNode& child,
                            std::string* error) {
        if (!parent || !error) return false;
        error->clear();
        if (!isCompositeKind(parent->kind)) {
            *error = "parent_is_not_composite";
            return false;
        }
        if (child.nodeId.empty()) {
            *error = "child_node_id_missing";
            return false;
        }
        parent->children.push_back(child);
        return true;
    }

    static std::vector<MemoryInspectorRow> collectVisibleRows(const MemoryValueNode& root,
                                                              std::size_t maxRows) {
        std::vector<MemoryInspectorRow> rows;
        if (maxRows == 0) return rows;
        appendNodeRows(root, 0, maxRows, &rows);
        return rows;
    }

    static std::string typeNameForKind(MemoryValueKind kind) {
        switch (kind) {
            case MemoryValueKind::Integer: return "int";
            case MemoryValueKind::Floating: return "float";
            case MemoryValueKind::Boolean: return "bool";
            case MemoryValueKind::String: return "string";
            case MemoryValueKind::Pointer: return "pointer";
            case MemoryValueKind::Container: return "container";
            case MemoryValueKind::Struct: return "struct";
        }
        return "unknown";
    }

    static bool resolvePointerRegion(const MemorySnapshot& snapshot,
                                     const MemoryValueNode& pointerNode,
                                     const MemoryRegionRecord** outRegion,
                                     std::string* error) {
        if (!outRegion || !error) return false;
        *outRegion = nullptr;
        error->clear();
        if (pointerNode.kind != MemoryValueKind::Pointer) {
            *error = "node_is_not_pointer";
            return false;
        }

        std::uint64_t address = 0;
        if (!parseAddress(pointerNode.valueText, &address)) {
            *error = "pointer_value_parse_failed";
            return false;
        }

        *outRegion = MemorySnapshotModel::findRegionByAddress(snapshot, address);
        return *outRegion != nullptr;
    }

private:
    static bool isCompositeKind(MemoryValueKind kind) {
        return kind == MemoryValueKind::Container || kind == MemoryValueKind::Struct;
    }

    static bool parseAddress(const std::string& text, std::uint64_t* outAddress) {
        if (!outAddress || text.empty()) return false;
        try {
            size_t parsed = 0;
            int base = 10;
            if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) base = 16;
            const auto value = std::stoull(text, &parsed, base);
            if (parsed != text.size() || value == 0) return false;
            *outAddress = static_cast<std::uint64_t>(value);
            return true;
        } catch (...) {
            return false;
        }
    }

    static void appendNodeRows(const MemoryValueNode& node,
                               int depth,
                               std::size_t maxRows,
                               std::vector<MemoryInspectorRow>* rows) {
        if (rows->size() >= maxRows) return;

        MemoryInspectorRow row;
        row.nodeId = node.nodeId;
        row.label = node.label;
        row.typeName = typeNameForKind(node.kind);
        row.valueText = node.valueText;
        row.depth = depth;
        row.expandable = isCompositeKind(node.kind);
        row.expanded = node.expanded;
        rows->push_back(row);

        if (!row.expandable || !node.expanded) return;
        for (const auto& child : node.children) {
            appendNodeRows(child, depth + 1, maxRows, rows);
            if (rows->size() >= maxRows) return;
        }
    }
};
