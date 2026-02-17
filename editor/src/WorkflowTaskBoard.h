#pragma once

#include "WorkflowState.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct TaskBoardFilter {
    std::string workerType;
    std::string priority;
    std::string language;
    std::string fileContains;
};

struct TaskCard {
    std::string itemId;
    std::string title;
    std::string workerType;
    std::string workerIcon;
    std::string priority;
    std::string language;
    std::string file;
    std::string status;
    std::vector<std::string> tags;
};

struct TaskBoardColumn {
    std::string key;
    std::string label;
    std::vector<TaskCard> cards;
};

class WorkflowTaskBoard {
public:
    static std::vector<TaskBoardColumn> build(const WorkflowState& wf,
                                              const TaskBoardFilter& filter = {}) {
        std::map<std::string, TaskBoardColumn> cols;
        cols["pending"] = {"pending", "Pending", {}};
        cols["ready"] = {"ready", "Ready", {}};
        cols["in-progress"] = {"in-progress", "In Progress", {}};
        cols["review"] = {"review", "Review", {}};
        cols["complete"] = {"complete", "Complete", {}};

        for (const auto& status : {WI_PENDING, WI_READY, WI_ASSIGNED, WI_IN_PROGRESS,
                                    WI_REVIEW, WI_COMPLETE, WI_REJECTED}) {
            for (const auto& wi : wf.queue.getByStatus(status)) {
                TaskCard card = toCard(wi);
                if (!matchesFilter(card, filter)) continue;
                std::string col = normalizeColumnKey(wi.status);
                if (cols.find(col) == cols.end()) continue;
                cols[col].cards.push_back(card);
            }
        }

        std::vector<TaskBoardColumn> out;
        for (const auto& k : {"pending", "ready", "in-progress", "review", "complete"}) {
            auto& c = cols[k];
            std::sort(c.cards.begin(), c.cards.end(),
                      [](const TaskCard& a, const TaskCard& b) {
                          if (priorityToInt(a.priority) != priorityToInt(b.priority)) {
                              return priorityToInt(a.priority) < priorityToInt(b.priority);
                          }
                          return a.itemId < b.itemId;
                      });
            out.push_back(c);
        }
        return out;
    }

    static bool moveItem(WorkflowState& wf, const std::string& itemId,
                         const std::string& targetColumn) {
        auto itemOpt = wf.queue.getItem(itemId);
        if (!itemOpt.has_value()) return false;
        std::string status = columnToStatus(targetColumn);
        if (status.empty()) return false;
        WorkItem item = *itemOpt;
        item.status = status;
        return wf.queue.updateItem(itemId, item);
    }

private:
    static TaskCard toCard(const WorkItem& wi) {
        TaskCard c;
        c.itemId = wi.id;
        c.title = wi.nodeName.empty() ? wi.nodeId : wi.nodeName;
        c.workerType = wi.workerType;
        c.workerIcon = iconForWorker(wi.workerType);
        c.priority = wi.priority;
        c.language = languageFromPath(wi.bufferId);
        c.file = wi.bufferId;
        c.status = normalizeColumnKey(wi.status);
        c.tags = tagsForItem(wi);
        return c;
    }

    static std::vector<std::string> tagsForItem(const WorkItem& wi) {
        std::vector<std::string> tags;
        if (!wi.contextWidth.empty()) tags.push_back("ctx:" + wi.contextWidth);
        if (wi.reviewRequired) tags.push_back("review");
        if (!wi.dependencies.empty()) tags.push_back("deps:" + std::to_string(wi.dependencies.size()));
        return tags;
    }

    static std::string normalizeColumnKey(const std::string& status) {
        if (status == WI_ASSIGNED || status == WI_IN_PROGRESS) return "in-progress";
        if (status == WI_REJECTED) return "pending";
        if (status == WI_PENDING) return "pending";
        if (status == WI_READY) return "ready";
        if (status == WI_REVIEW) return "review";
        if (status == WI_COMPLETE) return "complete";
        return "";
    }

    static std::string columnToStatus(const std::string& column) {
        if (column == "pending") return WI_PENDING;
        if (column == "ready") return WI_READY;
        if (column == "in-progress") return WI_IN_PROGRESS;
        if (column == "review") return WI_REVIEW;
        if (column == "complete") return WI_COMPLETE;
        return "";
    }

    static std::string iconForWorker(const std::string& worker) {
        if (worker == "deterministic" || worker == "template") return "D";
        if (worker == "slm") return "S";
        if (worker == "llm") return "L";
        if (worker == "human") return "H";
        return "?";
    }

    static std::string languageFromPath(const std::string& path) {
        auto dot = path.find_last_of('.');
        if (dot == std::string::npos) return "";
        std::string ext = path.substr(dot);
        if (ext == ".cpp" || ext == ".cc" || ext == ".hpp" || ext == ".h") return "cpp";
        if (ext == ".py") return "python";
        if (ext == ".js") return "javascript";
        if (ext == ".ts") return "typescript";
        if (ext == ".java") return "java";
        if (ext == ".rs") return "rust";
        if (ext == ".go") return "go";
        return "";
    }

    static bool matchesFilter(const TaskCard& card, const TaskBoardFilter& f) {
        if (!f.workerType.empty() && card.workerType != f.workerType) return false;
        if (!f.priority.empty() && card.priority != f.priority) return false;
        if (!f.language.empty() && card.language != f.language) return false;
        if (!f.fileContains.empty() && card.file.find(f.fileContains) == std::string::npos) return false;
        return true;
    }
};
