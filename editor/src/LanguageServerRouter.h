#pragma once
#include <map>
#include <string>

namespace whetstone {

class LanguageServerRouter {
public:
    LanguageServerRouter() {
        extMap_[".py"]  = "Python";
        extMap_[".rs"]  = "Rust";
        extMap_[".go"]  = "Go";
        extMap_[".ts"]  = "TypeScript";
        extMap_[".js"]  = "TypeScript";
        extMap_[".cpp"] = "C++";
        extMap_[".cc"]  = "C++";
        extMap_[".cxx"] = "C++";
        extMap_[".h"]   = "C++";
        extMap_[".hpp"] = "C++";

        langMap_["python"]     = "Python";
        langMap_["rust"]       = "Rust";
        langMap_["go"]         = "Go";
        langMap_["typescript"] = "TypeScript";
        langMap_["javascript"] = "TypeScript";
        langMap_["cpp"]        = "C++";
        langMap_["c++"]        = "C++";
    }

    std::string routeByUri(const std::string& uri) const {
        auto dot = uri.rfind('.');
        if (dot == std::string::npos) return "unknown";
        std::string ext = uri.substr(dot);
        auto it = extMap_.find(ext);
        if (it == extMap_.end()) return "unknown";
        return it->second;
    }

    std::string routeByLanguageId(const std::string& languageId) const {
        std::string lower = languageId;
        for (auto& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        auto it = langMap_.find(lower);
        if (it == langMap_.end()) return "unknown";
        return it->second;
    }

private:
    std::map<std::string, std::string> extMap_;
    std::map<std::string, std::string> langMap_;
};

} // namespace whetstone
