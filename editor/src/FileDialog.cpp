#include "FileDialog.h"

#ifdef _WIN32
#define TINYFD_WIN_CONSOLE
#endif

#include "tinyfiledialogs.h"

static const char* const* buildFilterArray(const std::vector<std::string>& filters, std::vector<const char*>& out) {
    out.clear();
    for (const auto& f : filters) out.push_back(f.c_str());
    return out.empty() ? nullptr : out.data();
}

std::string FileDialog::openFileNative(const OpenRequest& req) {
    std::vector<const char*> filters;
    const char* const* filter = buildFilterArray(req.filters, filters);
    const char* path = tinyfd_openFileDialog(
        req.title.empty() ? "Open File" : req.title.c_str(),
        req.defaultPath.empty() ? nullptr : req.defaultPath.c_str(),
        (int)req.filters.size(),
        filter,
        nullptr,
        0
    );
    return path ? std::string(path) : std::string();
}

std::string FileDialog::saveFileNative(const SaveRequest& req) {
    std::vector<const char*> filters;
    const char* const* filter = buildFilterArray(req.filters, filters);
    const char* path = tinyfd_saveFileDialog(
        req.title.empty() ? "Save File" : req.title.c_str(),
        req.defaultPath.empty() ? nullptr : req.defaultPath.c_str(),
        (int)req.filters.size(),
        filter,
        nullptr
    );
    return path ? std::string(path) : std::string();
}

std::string FileDialog::openFolderNative(const FolderRequest& req) {
    const char* path = tinyfd_selectFolderDialog(
        req.title.empty() ? "Open Folder" : req.title.c_str(),
        req.defaultPath.empty() ? nullptr : req.defaultPath.c_str()
    );
    return path ? std::string(path) : std::string();
}
