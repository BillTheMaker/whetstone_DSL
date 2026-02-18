#pragma once
// Step 649: AppImage/.deb package generator

#include <string>

struct PackageScripts {
    bool success = false;
    std::string appImageScript;
    std::string debianControl;
};

class PackageScriptGenerator {
public:
    static PackageScripts generate(const std::string& appName,
                                   const std::string& version) {
        PackageScripts out;
        if (appName.empty() || version.empty()) return out;
        out.success = true;
        out.appImageScript = "#!/usr/bin/env bash\n"
                             "set -euo pipefail\n"
                             "appimagetool AppDir " + appName + "-" + version + ".AppImage\n";
        out.debianControl = "Package: " + appName + "\n"
                            "Version: " + version + "\n"
                            "Architecture: amd64\n"
                            "Maintainer: Whetstone\n"
                            "Description: Whetstone editor package\n";
        return out;
    }
};
