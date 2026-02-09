#pragma once
// Step 88: Drag-and-drop handler

#include <string>
#include <functional>
#include <filesystem>

class DragDropHandler {
public:
    using FileCallback = std::function<void(const std::string&)>;
    using FolderCallback = std::function<void(const std::string&)>;

    static void handleDrop(const std::string& path,
                           const FileCallback& onFile,
                           const FolderCallback& onFolder) {
        if (path.empty()) return;
        std::filesystem::path p(path);
        if (std::filesystem::is_directory(p)) {
            if (onFolder) onFolder(path);
        } else {
            if (onFile) onFile(path);
        }
    }
};
