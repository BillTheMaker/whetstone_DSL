#pragma once
// Step 555: Breakpoint System

#include <map>
#include <set>
#include <string>
#include <vector>

struct Breakpoint {
    std::string id;
    std::string filePath;
    int line = 0;
    bool enabled = true;
    std::string condition;
    int hitCountTarget = 0;
    int currentHits = 0;
};

class BreakpointSystem {
public:
    bool addLineBreakpoint(const std::string& id,
                           const std::string& filePath,
                           int line) {
        return addBreakpoint({id, filePath, line, true, "", 0, 0});
    }

    bool addConditionalBreakpoint(const std::string& id,
                                  const std::string& filePath,
                                  int line,
                                  const std::string& condition) {
        return addBreakpoint({id, filePath, line, true, condition, 0, 0});
    }

    bool addHitCountBreakpoint(const std::string& id,
                               const std::string& filePath,
                               int line,
                               int hitCountTarget) {
        if (hitCountTarget <= 0) return false;
        return addBreakpoint({id, filePath, line, true, "", hitCountTarget, 0});
    }

    bool enable(const std::string& id) {
        auto* bp = findMutable(id);
        if (!bp) return false;
        bp->enabled = true;
        return true;
    }

    bool disable(const std::string& id) {
        auto* bp = findMutable(id);
        if (!bp) return false;
        bp->enabled = false;
        return true;
    }

    bool remove(const std::string& id) {
        auto it = index_.find(id);
        if (it == index_.end()) return false;
        breakpoints_.erase(breakpoints_.begin() + static_cast<long>(it->second));
        index_.erase(it);
        rebuildIndex();
        return true;
    }

    bool shouldBreak(const std::string& id, bool conditionEvaluatedTrue) {
        auto* bp = findMutable(id);
        if (!bp || !bp->enabled) return false;

        if (!bp->condition.empty() && !conditionEvaluatedTrue) return false;

        if (bp->hitCountTarget > 0) {
            ++bp->currentHits;
            return bp->currentHits >= bp->hitCountTarget;
        }

        return true;
    }

    void reconcileOnFileEdit(const std::string& filePath,
                             int fromLine,
                             int lineDelta) {
        for (auto& bp : breakpoints_) {
            if (bp.filePath != filePath) continue;
            if (bp.line >= fromLine) {
                bp.line += lineDelta;
                if (bp.line < 1) bp.line = 1;
            }
        }
        rebuildIndex();
    }

    std::vector<Breakpoint> list() const { return breakpoints_; }

private:
    std::vector<Breakpoint> breakpoints_;
    std::map<std::string, size_t> index_;

    bool addBreakpoint(const Breakpoint& bp) {
        if (bp.id.empty() || bp.filePath.empty() || bp.line <= 0) return false;
        if (index_.count(bp.id) != 0) return false;
        breakpoints_.push_back(bp);
        index_[bp.id] = breakpoints_.size() - 1;
        return true;
    }

    Breakpoint* findMutable(const std::string& id) {
        auto it = index_.find(id);
        if (it == index_.end()) return nullptr;
        return &breakpoints_[it->second];
    }

    void rebuildIndex() {
        index_.clear();
        for (size_t i = 0; i < breakpoints_.size(); ++i) {
            index_[breakpoints_[i].id] = i;
        }
    }
};
