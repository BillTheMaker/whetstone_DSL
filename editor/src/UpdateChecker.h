#pragma once
// Step 164: Update checker (stub)

#include <string>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct UpdateInfo {
    std::string version;
    std::string url;
    std::string notes;
    bool available = false;
};

class UpdateChecker {
public:
    UpdateInfo check(const std::string& updateUrl) {
        UpdateInfo info;
        if (updateUrl.empty()) return info;
        if (startsWith(updateUrl, "file://")) {
            std::string path = updateUrl.substr(7);
            std::ifstream in(path);
            if (!in.is_open()) return info;
            std::ostringstream ss;
            ss << in.rdbuf();
            parseManifest(ss.str(), info);
            return info;
        }
        // Network not implemented; return stub.
        return info;
    }

private:
    static bool startsWith(const std::string& s, const std::string& prefix) {
        return s.rfind(prefix, 0) == 0;
    }

    static void parseManifest(const std::string& text, UpdateInfo& out) {
        try {
            json j = json::parse(text);
            out.version = j.value("version", "");
            out.url = j.value("url", "");
            out.notes = j.value("notes", "");
            out.available = !out.version.empty();
        } catch (...) {
            out.available = false;
        }
    }
};
