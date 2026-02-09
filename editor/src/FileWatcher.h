#pragma once
// Step 89: File watcher (polling)

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <chrono>

class FileWatcher {
public:
    void watch(const std::string& path) {
        if (path.empty()) return;
        auto p = std::filesystem::path(path);
        if (!std::filesystem::exists(p)) return;
        files_[path] = std::filesystem::last_write_time(p);
    }

    void unwatch(const std::string& path) {
        files_.erase(path);
    }

    std::vector<std::string> poll() {
        std::vector<std::string> changed;
        auto now = std::chrono::steady_clock::now();
        if (now - lastPoll_ < std::chrono::seconds(2)) return changed;
        lastPoll_ = now;

        for (auto& kv : files_) {
            const std::string& path = kv.first;
            auto p = std::filesystem::path(path);
            if (!std::filesystem::exists(p)) continue;
            auto ts = std::filesystem::last_write_time(p);
            if (ts != kv.second) {
                kv.second = ts;
                changed.push_back(path);
            }
        }
        return changed;
    }

private:
    std::unordered_map<std::string, std::filesystem::file_time_type> files_;
    std::chrono::steady_clock::time_point lastPoll_ = std::chrono::steady_clock::now() - std::chrono::seconds(10);
};
