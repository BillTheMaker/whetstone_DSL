#pragma once
// Step 299: Cross-type annotation conflict detection
//
// Detects conflicts between different annotation types on the same node.
// This extends AnnotationConflict.h which handles same-family parent/child
// conflicts. This file detects semantic conflicts across annotation families.

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "ast/ASTNode.h"
#include "ast/Annotation.h"
#include "AnnotationConflict.h"

struct CrossTypeConflict {
    std::string type1;
    std::string type2;
    std::string message;
    std::string nodeId;
};

inline void collectCrossTypeConflicts(const ASTNode* node,
                                       std::vector<CrossTypeConflict>& out) {
    if (!node) return;
    auto annos = node->getChildren("annotations");

    // Build a set of annotation types present on this node
    std::map<std::string, const ASTNode*> present;
    for (const auto* a : annos) {
        present[a->conceptType] = a;
    }

    // Check conflict pairs
    auto checkPair = [&](const std::string& t1, const std::string& t2,
                         const std::string& msg,
                         std::function<bool()> condition = nullptr) {
        if (present.count(t1) && present.count(t2)) {
            if (!condition || condition()) {
                out.push_back({t1, t2, msg, node->id});
            }
        }
    };

    // 1. @Pure + @Blocking → "Pure function cannot be blocking"
    checkPair("PureAnnotation", "BlockingAnnotation",
              "Pure function cannot be blocking");

    // 2. @Atomic + @Sync → "Atomic operations conflict with sync primitives"
    checkPair("AtomicAnnotation", "SyncAnnotation",
              "Atomic operations conflict with sync primitives");

    // 3. @Capture(move) + @Owner(Shared_ARC) → "Move capture conflicts with shared ownership"
    checkPair("CaptureAnnotation", "OwnerAnnotation",
              "Move capture conflicts with shared ownership",
              [&]() {
                  auto* cap = static_cast<const CaptureAnnotation*>(present["CaptureAnnotation"]);
                  auto* own = static_cast<const OwnerAnnotation*>(present["OwnerAnnotation"]);
                  return cap->strategy == "move" && own->strategy == "Shared_ARC";
              });

    // 4. @ConstExpr + @Exec(async) → "Compile-time eval conflicts with async execution"
    checkPair("ConstExprAnnotation", "ExecAnnotation",
              "Compile-time eval conflicts with async execution",
              [&]() {
                  auto* exec = static_cast<const ExecAnnotation*>(present["ExecAnnotation"]);
                  return exec->mode == "async";
              });

    // 5. @Inline(Always) + @TailCall → "Always-inline conflicts with tail call optimization"
    checkPair("InlineAnnotation", "TailCallAnnotation",
              "Always-inline conflicts with tail call optimization",
              [&]() {
                  auto* inl = static_cast<const InlineAnnotation*>(present["InlineAnnotation"]);
                  return inl->mode == "Always";
              });

    // 6. @Pack + @Layout(aligned) → "Packed layout conflicts with alignment"
    checkPair("PackAnnotation", "LayoutAnnotation",
              "Packed layout conflicts with alignment",
              [&]() {
                  auto* lay = static_cast<const LayoutAnnotation*>(present["LayoutAnnotation"]);
                  return lay->mode == "aligned";
              });

    // Recurse into children
    for (auto* child : node->allChildren()) {
        collectCrossTypeConflicts(child, out);
    }
}
