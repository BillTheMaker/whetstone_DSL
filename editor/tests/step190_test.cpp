// Step 190: Vulnerability database parsing/querying.

#include <cassert>
#include <fstream>
#include <filesystem>
#include "VulnerabilityDatabase.h"

static void writeFile(const std::string& path, const std::string& contents) {
    std::ofstream out(path);
    out << contents;
}

int main() {
    std::filesystem::path cacheDir = "tmp_vuln_cache";
    std::filesystem::create_directories(cacheDir);

    const std::string json = R"({
        "vulnerabilities": [{
            "id": "CVE-2024-0001",
            "summary": "Test vuln",
            "severity": [{"type": "CVSS_V3", "score": "7.5"}],
            "aliases": ["CVE-2024-0001"],
            "affected": [{
                "package": {"ecosystem": "PyPI", "name": "requests"},
                "ranges": [{
                    "type": "SEMVER",
                    "events": [
                        {"introduced": "1.0.0"},
                        {"fixed": "2.0.0"}
                    ]
                }],
                "versions": ["1.5.0"]
            }]
        }]
    })";

    const std::string fileName = "PyPI_requests.json";
    writeFile((cacheDir / fileName).string(), json);

    VulnerabilityDatabase db;
    db.setCacheDirectory(cacheDir.string());

    auto hit = db.query("PyPI", "requests", "1.5.0");
    assert(hit.size() == 1);
    assert(hit[0].cveId == "CVE-2024-0001");
    assert(hit[0].severity == "High");

    auto miss = db.query("PyPI", "requests", "2.1.0");
    assert(miss.empty());

    printf("step190_test: all assertions passed\n");
    return 0;
}
