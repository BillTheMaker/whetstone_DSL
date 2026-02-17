#pragma once
// Step 560: Locals + Watches Panel Data Model

#include <map>
#include <string>
#include <vector>

struct LocalVarEntry {
    std::string name;
    std::string type;
    std::string value;
};

struct WatchExpression {
    std::string id;
    std::string expression;
    bool enabled = true;
    std::string lastValue;
    bool lastEvalOk = false;
};

class LocalsWatchesPanelModel {
public:
    void setLocals(const std::vector<LocalVarEntry>& locals) {
        locals_ = locals;
    }

    const std::vector<LocalVarEntry>& locals() const { return locals_; }

    bool addWatch(const std::string& id, const std::string& expression) {
        if (id.empty() || expression.empty()) return false;
        if (watchIndex_.count(id) != 0) return false;
        watches_.push_back({id, expression, true, "", false});
        watchIndex_[id] = watches_.size() - 1;
        return true;
    }

    bool removeWatch(const std::string& id) {
        auto it = watchIndex_.find(id);
        if (it == watchIndex_.end()) return false;
        watches_.erase(watches_.begin() + static_cast<long>(it->second));
        rebuildIndex();
        return true;
    }

    bool enableWatch(const std::string& id, bool enabled) {
        auto* w = findWatch(id);
        if (!w) return false;
        w->enabled = enabled;
        return true;
    }

    bool updateWatchValue(const std::string& id,
                          const std::string& value,
                          bool evalOk) {
        auto* w = findWatch(id);
        if (!w) return false;
        w->lastValue = value;
        w->lastEvalOk = evalOk;
        return true;
    }

    const std::vector<WatchExpression>& watches() const { return watches_; }

    std::vector<WatchExpression> activeWatches() const {
        std::vector<WatchExpression> out;
        for (const auto& w : watches_) {
            if (w.enabled) out.push_back(w);
        }
        return out;
    }

private:
    std::vector<LocalVarEntry> locals_;
    std::vector<WatchExpression> watches_;
    std::map<std::string, size_t> watchIndex_;

    WatchExpression* findWatch(const std::string& id) {
        auto it = watchIndex_.find(id);
        if (it == watchIndex_.end()) return nullptr;
        return &watches_[it->second];
    }

    void rebuildIndex() {
        watchIndex_.clear();
        for (size_t i = 0; i < watches_.size(); ++i) {
            watchIndex_[watches_[i].id] = i;
        }
    }
};
