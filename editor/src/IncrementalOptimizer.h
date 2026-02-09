#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "ast/ASTNode.h"
#include "ast/Expression.h"
#include "ast/Statement.h"

class IncrementalOptimizer {
public:
    struct TransformRecord {
        std::string transformId;
        std::string transformName;
        std::vector<std::string> affectedNodeIds;
        std::string timestamp;
    };

    void setRoot(ASTNode* root) { root_ = root; }

    std::string applyTransform(const std::string& transformName) {
        std::string tid = "t" + std::to_string(nextId_++);

        InternalRecord record;
        record.info.transformId = tid;
        record.info.transformName = transformName;
        record.info.timestamp = currentTimestamp();

        if (transformName == "constant-fold") {
            applyConstantFolding(root_, tid, record);
        } else if (transformName == "dead-code-elim") {
            applyDeadCodeElim(root_, tid, record);
        }

        history_.push_back(std::move(record));
        return tid;
    }

    std::vector<TransformRecord> getTransformHistory() const {
        std::vector<TransformRecord> result;
        for (const auto& r : history_) {
            result.push_back(r.info);
        }
        return result;
    }

    bool undoTransform(const std::string& transformId) {
        for (auto it = history_.begin(); it != history_.end(); ++it) {
            if (it->info.transformId == transformId) {
                // Apply undo actions in reverse order
                for (auto ait = it->undoActions.rbegin(); ait != it->undoActions.rend(); ++ait) {
                    ait->undo();
                }
                // Remove provenance entries for this transform
                for (const auto& nodeId : it->info.affectedNodeIds) {
                    provenance_.erase(nodeId);
                }
                history_.erase(it);
                return true;
            }
        }
        return false;
    }

    bool undoLast() {
        if (history_.empty()) return false;
        return undoTransform(history_.back().info.transformId);
    }

    std::string getProvenance(const std::string& nodeId) const {
        auto it = provenance_.find(nodeId);
        return it != provenance_.end() ? it->second : "";
    }

    static std::string currentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        return oss.str();
    }

private:
    ASTNode* root_ = nullptr;
    int nextId_ = 1;

    struct UndoAction {
        std::function<void()> undo;
    };

    struct InternalRecord {
        TransformRecord info;
        std::vector<UndoAction> undoActions;
    };

    std::vector<InternalRecord> history_;
    std::map<std::string, std::string> provenance_; // nodeId → transformId

    void applyConstantFolding(ASTNode* node, const std::string& tid, InternalRecord& record) {
        // Bottom-up: process children first
        for (const auto& role : node->childRoles()) {
            auto children = node->getChildren(role);
            for (auto* child : children) {
                applyConstantFolding(child, tid, record);
            }
        }

        // Check each child role for foldable BinaryOperations
        for (const auto& role : node->childRoles()) {
            auto children = node->getChildren(role);
            for (size_t i = 0; i < children.size(); ++i) {
                auto* child = children[i];
                if (child->conceptType == "BinaryOperation") {
                    auto* binOp = static_cast<BinaryOperation*>(child);
                    auto* leftNode = binOp->getChild("left");
                    auto* rightNode = binOp->getChild("right");

                    if (leftNode && rightNode &&
                        leftNode->conceptType == "IntegerLiteral" &&
                        rightNode->conceptType == "IntegerLiteral") {

                        auto* leftLit = static_cast<IntegerLiteral*>(leftNode);
                        auto* rightLit = static_cast<IntegerLiteral*>(rightNode);
                        int foldedValue = evaluateOp(binOp->op, leftLit->value, rightLit->value);

                        auto* replacement = new IntegerLiteral("folded_" + binOp->id, foldedValue);

                        // Record undo: restore old child
                        ASTNode* parent = node;
                        std::string savedRole = role;
                        ASTNode* oldChild = child;
                        record.undoActions.push_back({[parent, savedRole, oldChild]() {
                            parent->setChild(savedRole, oldChild);
                        }});

                        node->setChild(role, replacement);

                        // Track provenance and affected nodes
                        provenance_[replacement->id] = tid;
                        record.info.affectedNodeIds.push_back(replacement->id);
                    }
                }
            }
        }
    }

    void applyDeadCodeElim(ASTNode* node, const std::string& tid, InternalRecord& record) {
        if (node->conceptType == "Function") {
            auto body = node->getChildren("body");
            bool foundReturn = false;
            std::vector<ASTNode*> toRemove;

            for (auto* stmt : body) {
                if (foundReturn) {
                    toRemove.push_back(stmt);
                }
                if (stmt->conceptType == "Return") {
                    foundReturn = true;
                }
            }

            for (auto* dead : toRemove) {
                // Record undo: re-add the removed child
                ASTNode* parent = node;
                ASTNode* removedChild = dead;
                record.undoActions.push_back({[parent, removedChild]() {
                    parent->addChild("body", removedChild);
                }});

                node->removeChild(dead);
                record.info.affectedNodeIds.push_back(dead->id);
            }
        }

        // Recurse
        for (auto* child : node->allChildren()) {
            applyDeadCodeElim(child, tid, record);
        }
    }

    int evaluateOp(const std::string& op, int left, int right) {
        if (op == "+") return left + right;
        if (op == "-") return left - right;
        if (op == "*") return left * right;
        if (op == "/" && right != 0) return left / right;
        if (op == "%") return left % right;
        return 0;
    }
};
