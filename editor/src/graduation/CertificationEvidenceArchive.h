#pragma once
// Step 841: Certification evidence archive format v2.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct EvidenceRecord {
    std::string recordId;
    std::string pairId;
    std::string cycleId;
    std::string artifactPath;
    std::string checksum;
    std::string archivedAt;
};

struct EvidenceArchiveV2 {
    std::string archiveId;
    std::string version = "v2";
    std::vector<EvidenceRecord> records;
    int totalRecords = 0;
};

class CertificationEvidenceArchive {
public:
    static EvidenceArchiveV2 build(const std::string& archiveId,
                                   const std::vector<EvidenceRecord>& records) {
        EvidenceArchiveV2 a;
        a.archiveId = archiveId;
        a.records = records;
        a.totalRecords = static_cast<int>(records.size());
        return a;
    }

    static bool validate(const EvidenceArchiveV2& a, std::string* error = nullptr) {
        if (a.archiveId.empty()) { if (error) *error = "archive_id_missing"; return false; }
        return true;
    }

    static nlohmann::json toJson(const EvidenceArchiveV2& a) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& r : a.records)
            arr.push_back({{"record_id", r.recordId}, {"pair_id", r.pairId},
                           {"cycle_id", r.cycleId}, {"artifact", r.artifactPath}});
        return {{"archive_id", a.archiveId}, {"version", a.version},
                {"total", a.totalRecords}, {"records", arr}};
    }
};
