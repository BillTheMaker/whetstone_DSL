#pragma once
// Step 650: HiveMind-push auto-update mechanism model

#include <string>

struct UpdateJob {
    std::string version;
    std::string downloadUrl;
    std::string expectedSha256;
};

struct UpdateExecutionResult {
    bool accepted = false;
    bool hashVerified = false;
    bool binaryReplaced = false;
    bool restartScheduled = false;
};

class HiveMindAutoUpdateModel {
public:
    static bool isValidJob(const UpdateJob& job) {
        return !job.version.empty() && !job.downloadUrl.empty() && !job.expectedSha256.empty();
    }

    static bool verifyHash(const std::string& actualSha256,
                           const std::string& expectedSha256) {
        return !actualSha256.empty() && actualSha256 == expectedSha256;
    }

    static UpdateExecutionResult execute(const UpdateJob& job,
                                         const std::string& downloadedSha256) {
        UpdateExecutionResult out;
        out.accepted = isValidJob(job);
        if (!out.accepted) return out;
        out.hashVerified = verifyHash(downloadedSha256, job.expectedSha256);
        if (!out.hashVerified) return out;
        out.binaryReplaced = true;
        out.restartScheduled = true;
        return out;
    }
};
