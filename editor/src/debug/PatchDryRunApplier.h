#pragma once
// Step 1460: patch dry-run applier.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct PatchDryRunResult {
    bool validDiff = false;
    bool filesResolvable = false;
    bool success = false;
    std::vector<std::string> touchedFiles;
    std::vector<std::string> errors;
};

class PatchDryRunApplier {
public:
    static PatchDryRunResult run(const std::string& diffText,
                                 const std::string& workspaceRoot = "") {
        (void)workspaceRoot;
        PatchDryRunResult r;
        if (diffText.find("diff --git ") == std::string::npos) {
            r.errors.push_back("diff_missing_header");
            return r;
        }
        r.validDiff = true;

        size_t p = 0;
        while (true) {
            p = diffText.find("diff --git a/", p);
            if (p == std::string::npos) break;
            size_t start = p + 13;
            size_t end = diffText.find(" b/", start);
            if (end == std::string::npos) break;
            std::string file = diffText.substr(start, end - start);
            if (!file.empty()) r.touchedFiles.push_back(file);
            p = end + 3;
        }
        if (r.touchedFiles.empty()) {
            r.errors.push_back("no_touched_files");
            return r;
        }
        r.filesResolvable = true;
        r.success = r.validDiff && r.filesResolvable;
        return r;
    }

    static nlohmann::json toJson(const PatchDryRunResult& r) {
        return {{"valid_diff", r.validDiff}, {"files_resolvable", r.filesResolvable}, {"success", r.success},
                {"touched_files", r.touchedFiles}, {"errors", r.errors}};
    }
};
