#pragma once
// Step 57: Buffer management
//
// BufferManager tracks multiple open file buffers. Each buffer has a path,
// content, language, and modified flag. One buffer is active at a time.
// Provides open, close, switch, and content access operations.

#include <string>
#include <vector>
#include <map>

class BufferManager {
public:
    enum class BufferMode {
        Structured,
        Text
    };

    struct BufferInfo {
        std::string path;
        std::string content;
        std::string language;
        bool modified = false;
        BufferMode mode = BufferMode::Structured;
    };

    BufferManager() = default;

    // Open a file into a buffer (makes it active)
    bool openBuffer(const std::string& path, const std::string& content,
                    const std::string& language,
                    BufferMode mode = BufferMode::Structured) {
        if (hasBuffer(path)) return false;  // already open
        BufferInfo info{path, content, language, false, mode};
        buffers_[path] = info;
        activeBuffer_ = path;
        return true;
    }

    // Close a buffer by path
    bool closeBuffer(const std::string& path) {
        auto it = buffers_.find(path);
        if (it == buffers_.end()) return false;
        buffers_.erase(it);
        // If we closed the active buffer, switch to another
        if (activeBuffer_ == path) {
            if (!buffers_.empty())
                activeBuffer_ = buffers_.begin()->first;
            else
                activeBuffer_.clear();
        }
        return true;
    }

    // Get current active buffer path
    std::string getActiveBufferPath() const { return activeBuffer_; }

    // Switch to a different buffer
    bool switchToBuffer(const std::string& path) {
        if (!hasBuffer(path)) return false;
        activeBuffer_ = path;
        return true;
    }

    // Get buffer content
    std::string getBufferContent(const std::string& path) const {
        auto it = buffers_.find(path);
        if (it == buffers_.end()) return "";
        return it->second.content;
    }

    // Set buffer content
    void setBufferContent(const std::string& path, const std::string& content) {
        auto it = buffers_.find(path);
        if (it != buffers_.end()) {
            it->second.content = content;
        }
    }

    void setBufferMode(const std::string& path, BufferMode mode) {
        auto it = buffers_.find(path);
        if (it != buffers_.end()) {
            it->second.mode = mode;
        }
    }

    BufferMode getBufferMode(const std::string& path) const {
        auto it = buffers_.find(path);
        if (it != buffers_.end()) return it->second.mode;
        return BufferMode::Structured;
    }

    // Get list of all open buffer paths
    std::vector<std::string> getOpenBuffers() const {
        std::vector<std::string> result;
        for (const auto& [path, _] : buffers_) {
            result.push_back(path);
        }
        return result;
    }

    // Check if a buffer exists
    bool hasBuffer(const std::string& path) const {
        return buffers_.find(path) != buffers_.end();
    }

    // Get buffer info
    BufferInfo getBufferInfo(const std::string& path) const {
        auto it = buffers_.find(path);
        if (it != buffers_.end()) return it->second;
        return {};
    }

    // Mark buffer as modified
    void setModified(const std::string& path, bool modified) {
        auto it = buffers_.find(path);
        if (it != buffers_.end()) {
            it->second.modified = modified;
        }
    }

    // Get number of open buffers
    size_t bufferCount() const { return buffers_.size(); }

private:
    std::map<std::string, BufferInfo> buffers_;
    std::string activeBuffer_;
};
