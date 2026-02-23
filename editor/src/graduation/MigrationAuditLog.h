#pragma once
// Step 904: Migration audit log — append-only event log for migration activity.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct MigrationAuditEntry {
    std::string entryId;    // "AE-<N>"
    std::string pairId;
    std::string event;
    std::string timestamp;  // ISO-8601 string supplied by caller
    std::string detail;
};

class MigrationAuditLog {
    std::vector<MigrationAuditEntry> entries_;
    int nextId_ = 1;

public:
    void append(const MigrationAuditEntry& entry) {
        entries_.push_back(entry);
    }

    const std::vector<MigrationAuditEntry>& entries() const { return entries_; }

    std::size_t count() const { return entries_.size(); }

    void clear() {
        entries_.clear();
        nextId_ = 1;
    }

    std::vector<MigrationAuditEntry> filterByPair(const std::string& pairId) const {
        std::vector<MigrationAuditEntry> out;
        for (const auto& e : entries_)
            if (e.pairId == pairId) out.push_back(e);
        return out;
    }

    // Factory: creates a new entry with a generated entryId.
    // The caller is responsible for inserting into the log via append().
    MigrationAuditEntry makeEntry(const std::string& pairId,
                                  const std::string& event,
                                  const std::string& detail) {
        MigrationAuditEntry e;
        e.entryId   = "AE-" + std::to_string(nextId_++);
        e.pairId    = pairId;
        e.event     = event;
        e.timestamp = "1970-01-01T00:00:00Z";  // caller should override
        e.detail    = detail;
        return e;
    }

    static nlohmann::json toJson(const MigrationAuditEntry& e) {
        return {
            {"entry_id",  e.entryId},
            {"pair_id",   e.pairId},
            {"event",     e.event},
            {"timestamp", e.timestamp},
            {"detail",    e.detail}
        };
    }
};
