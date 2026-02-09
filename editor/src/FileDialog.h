#pragma once
// Step 84: Native file dialogs wrapper
//
// Provides a small abstraction over native file dialogs (tinyfiledialogs)
// with an injectable provider for tests.

#include <string>
#include <vector>
#include <functional>

class FileDialog {
public:
    struct OpenRequest {
        std::string title;
        std::vector<std::string> filters;
        std::string defaultPath;
    };

    struct SaveRequest {
        std::string title;
        std::vector<std::string> filters;
        std::string defaultPath;
    };

    struct FolderRequest {
        std::string title;
        std::string defaultPath;
    };

    using OpenFn = std::function<std::string(const OpenRequest&)>;
    using SaveFn = std::function<std::string(const SaveRequest&)>;
    using FolderFn = std::function<std::string(const FolderRequest&)>;

    struct Provider {
        OpenFn openFile;
        SaveFn saveFile;
        FolderFn openFolder;
    };

    static void setProvider(const Provider& provider) { provider_() = provider; }
    static void clearProvider() { provider_() = Provider{}; }

    static std::string openFile(const OpenRequest& req) {
        if (provider_().openFile) return provider_().openFile(req);
        return openFileNative(req);
    }

    static std::string saveFile(const SaveRequest& req) {
        if (provider_().saveFile) return provider_().saveFile(req);
        return saveFileNative(req);
    }

    static std::string openFolder(const FolderRequest& req) {
        if (provider_().openFolder) return provider_().openFolder(req);
        return openFolderNative(req);
    }

private:
    static Provider& provider_() {
        static Provider p{};
        return p;
    }

    static std::string openFileNative(const OpenRequest& req);
    static std::string saveFileNative(const SaveRequest& req);
    static std::string openFolderNative(const FolderRequest& req);
};
