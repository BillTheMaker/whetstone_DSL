#pragma once
// Step 321: TaskQueue — Priority Queue with Dependencies
//
// Ordered queue that respects both priority levels and dependency chains.
// Items whose dependencies are unsatisfied stay blocked; ready items are
// ordered by priority then creation time.

#include "WorkItem.h"
#include <vector>
#include <optional>
#include <algorithm>
#include <map>

class TaskQueue {
public:
    // Add a work item to the queue
    void enqueue(WorkItem item) {
        // Set status based on whether dependencies exist
        if (item.dependencies.empty()) {
            if (item.status == WI_PENDING) {
                transitionWorkItem(item, WI_READY);
            }
        }
        // else stays pending until dependencies complete
        items_.push_back(std::move(item));
    }

    // Returns items whose dependencies are all complete, ordered by priority then createdAt
    std::vector<WorkItem> getReady() const {
        std::vector<WorkItem> ready;
        for (const auto& item : items_) {
            if (item.status == WI_READY) {
                ready.push_back(item);
            }
        }
        sortByPriority(ready);
        return ready;
    }

    // Next ready item without removing
    std::optional<WorkItem> peek() const {
        auto ready = getReady();
        if (ready.empty()) return std::nullopt;
        return ready.front();
    }

    // Remove and return next ready item, transition to assigned
    std::optional<WorkItem> dequeue() {
        auto ready = getReady();
        if (ready.empty()) return std::nullopt;

        std::string id = ready.front().id;
        for (auto& item : items_) {
            if (item.id == id) {
                transitionWorkItem(item, WI_ASSIGNED);
                return item;
            }
        }
        return std::nullopt;
    }

    // Mark complete, trigger dependency re-evaluation
    bool complete(const std::string& itemId) {
        auto* item = findItem(itemId);
        if (!item) return false;

        // Transition through necessary states to reach complete
        if (item->status == WI_IN_PROGRESS || item->status == WI_REVIEW) {
            transitionWorkItem(*item, WI_COMPLETE);
        } else {
            return false;
        }

        // Re-evaluate blocked items
        resolveDependencies();
        return true;
    }

    // Mark rejected, re-enqueue as ready with feedback
    bool reject(const std::string& itemId, const std::string& feedback) {
        auto* item = findItem(itemId);
        if (!item) return false;

        if (item->status != WI_REVIEW) return false;

        transitionWorkItem(*item, WI_REJECTED);
        item->result.reasoning = feedback;

        // Re-enqueue: rejected → ready
        transitionWorkItem(*item, WI_READY);
        return true;
    }

    // Items waiting on dependencies
    std::vector<WorkItem> getBlocked() const {
        std::vector<WorkItem> blocked;
        for (const auto& item : items_) {
            if (item.status == WI_PENDING && !item.dependencies.empty()) {
                blocked.push_back(item);
            }
        }
        return blocked;
    }

    // Filter by lifecycle status
    std::vector<WorkItem> getByStatus(const std::string& status) const {
        std::vector<WorkItem> result;
        for (const auto& item : items_) {
            if (item.status == status) {
                result.push_back(item);
            }
        }
        return result;
    }

    int size() const { return static_cast<int>(items_.size()); }

    int readyCount() const {
        int count = 0;
        for (const auto& item : items_) {
            if (item.status == WI_READY) ++count;
        }
        return count;
    }

    int blockedCount() const {
        int count = 0;
        for (const auto& item : items_) {
            if (item.status == WI_PENDING && !item.dependencies.empty()) ++count;
        }
        return count;
    }

    // Lookup by ID
    std::optional<WorkItem> getItem(const std::string& itemId) const {
        for (const auto& item : items_) {
            if (item.id == itemId) return item;
        }
        return std::nullopt;
    }

    // Replace item (for external status updates)
    bool updateItem(const std::string& itemId, const WorkItem& updated) {
        for (auto& item : items_) {
            if (item.id == itemId) {
                item = updated;
                // If item became complete, re-evaluate blocked dependencies
                if (updated.status == WI_COMPLETE) {
                    resolveDependencies();
                }
                return true;
            }
        }
        return false;
    }

private:
    std::vector<WorkItem> items_;

    WorkItem* findItem(const std::string& id) {
        for (auto& item : items_) {
            if (item.id == id) return &item;
        }
        return nullptr;
    }

    // Check if a given item ID is complete
    bool isComplete(const std::string& id) const {
        for (const auto& item : items_) {
            if (item.id == id) return item.status == WI_COMPLETE;
        }
        // Dependency not in queue — treat as satisfied
        return true;
    }

    // Scan blocked items and promote to ready if all deps complete
    void resolveDependencies() {
        for (auto& item : items_) {
            if (item.status != WI_PENDING) continue;
            if (item.dependencies.empty()) continue;

            bool allComplete = true;
            for (const auto& dep : item.dependencies) {
                if (!isComplete(dep)) {
                    allComplete = false;
                    break;
                }
            }
            if (allComplete) {
                transitionWorkItem(item, WI_READY);
            }
        }
    }

    // Sort by priority (lower int = higher priority), then by createdAt
    static void sortByPriority(std::vector<WorkItem>& items) {
        std::sort(items.begin(), items.end(), [](const WorkItem& a, const WorkItem& b) {
            int pa = priorityToInt(a.priority);
            int pb = priorityToInt(b.priority);
            if (pa != pb) return pa < pb;
            return a.createdAt < b.createdAt;
        });
    }
};
