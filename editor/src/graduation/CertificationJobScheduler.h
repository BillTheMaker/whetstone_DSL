#pragma once
// Step 839: Certification job scheduler.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct CertificationJob {
    std::string jobId;
    std::string pairId;
    std::string scheduledAt;
    std::string status; // "pending", "running", "done", "failed"
    int priority = 0;
};

class CertificationJobScheduler {
public:
    bool schedule(const CertificationJob& job, std::string* error = nullptr) {
        if (job.jobId.empty()) { if (error) *error = "job_id_missing"; return false; }
        if (job.pairId.empty()) { if (error) *error = "pair_id_missing"; return false; }
        for (const auto& j : jobs_)
            if (j.jobId == job.jobId) { if (error) *error = "job_duplicate"; return false; }
        jobs_.push_back(job);
        return true;
    }

    std::vector<CertificationJob> pending() const {
        std::vector<CertificationJob> r;
        for (const auto& j : jobs_) if (j.status == "pending") r.push_back(j);
        return r;
    }

    bool markRunning(const std::string& jobId, std::string* error = nullptr) {
        for (auto& j : jobs_) if (j.jobId == jobId) { j.status = "running"; return true; }
        if (error) *error = "job_not_found";
        return false;
    }

    bool complete(const std::string& jobId, bool success, std::string* error = nullptr) {
        for (auto& j : jobs_) if (j.jobId == jobId) {
            j.status = success ? "done" : "failed"; return true;
        }
        if (error) *error = "job_not_found";
        return false;
    }

    int total() const { return static_cast<int>(jobs_.size()); }

    static nlohmann::json toJson(const CertificationJob& j) {
        return {{"job_id", j.jobId}, {"pair_id", j.pairId}, {"status", j.status}};
    }

private:
    std::vector<CertificationJob> jobs_;
};
