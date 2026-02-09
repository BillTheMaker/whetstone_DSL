#pragma once
#include <string>
#include <vector>

struct PackageInfo {
    std::string name;
    std::string description;
    std::string homepageUrl;
    std::vector<std::string> versions;
    std::vector<std::string> dependencies;
};

enum class PackageEcosystem {
    Python,
    Npm,
    Rust,
    Go,
    Java,
    Cpp
};

class PackageRegistry {
public:
    static PackageInfo query(PackageEcosystem eco, const std::string& name) {
        switch (eco) {
            case PackageEcosystem::Python: return queryPyPi(name);
            case PackageEcosystem::Npm:    return queryNpm(name);
            case PackageEcosystem::Rust:   return queryCrates(name);
            case PackageEcosystem::Go:     return queryGo(name);
            case PackageEcosystem::Java:   return queryMaven(name);
            case PackageEcosystem::Cpp:    return queryCpp(name);
            default:                       return {};
        }
    }

private:
    static PackageInfo baseInfo(const std::string& name, const std::string& desc,
                                const std::string& url) {
        PackageInfo info;
        info.name = name;
        info.description = desc;
        info.homepageUrl = url;
        info.versions = {"0.1.0"};
        return info;
    }

    static PackageInfo queryPyPi(const std::string& name) {
        return baseInfo(name, "PyPI package (stub)", "https://pypi.org/project/" + name + "/");
    }
    static PackageInfo queryNpm(const std::string& name) {
        return baseInfo(name, "npm package (stub)", "https://www.npmjs.com/package/" + name);
    }
    static PackageInfo queryCrates(const std::string& name) {
        return baseInfo(name, "crates.io package (stub)", "https://crates.io/crates/" + name);
    }
    static PackageInfo queryGo(const std::string& name) {
        return baseInfo(name, "Go module (stub)", "https://pkg.go.dev/" + name);
    }
    static PackageInfo queryMaven(const std::string& name) {
        return baseInfo(name, "Maven package (stub)", "https://search.maven.org/search?q=" + name);
    }
    static PackageInfo queryCpp(const std::string& name) {
        return baseInfo(name, "C/C++ package (stub)", "https://vcpkg.io/en/packages.html");
    }
};
