#pragma once
// Step 564: Memory Snapshot Model

#include "DebugValidationUtil.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

enum class MemoryRegionKind {
    Stack,
    Heap,
    Global
};

struct MemoryRegionRecord {
    std::string id;
    MemoryRegionKind kind = MemoryRegionKind::Heap;
    std::uint64_t startAddress = 0;
    std::uint64_t sizeBytes = 0;
    bool readOnly = false;
};

struct MemoryReferenceRecord {
    std::uint64_t fromAddress = 0;
    std::uint64_t toAddress = 0;
    std::string label;
};

struct MemorySnapshot {
    std::string snapshotId;
    std::string sessionId;
    std::string threadId;
    std::uint64_t sequence = 0;
    std::vector<MemoryRegionRecord> regions;
    std::vector<MemoryReferenceRecord> references;
};

class MemorySnapshotModel {
public:
    static bool createSnapshot(const std::string& snapshotId,
                               const std::string& sessionId,
                               const std::string& threadId,
                               std::uint64_t sequence,
                               MemorySnapshot* outSnapshot,
                               std::string* error) {
        if (!outSnapshot || !error) return false;
        error->clear();
        if (!hasRequiredDebugIds(snapshotId, sessionId)) {
            *error = "snapshot_id_or_session_id_missing";
            return false;
        }
        if (threadId.empty()) {
            *error = "thread_id_missing";
            return false;
        }

        MemorySnapshot snap;
        snap.snapshotId = snapshotId;
        snap.sessionId = sessionId;
        snap.threadId = threadId;
        snap.sequence = sequence;
        *outSnapshot = snap;
        return true;
    }

    static bool addRegion(MemorySnapshot* snapshot,
                          const MemoryRegionRecord& region,
                          std::string* error) {
        if (!snapshot || !error) return false;
        error->clear();
        if (region.id.empty()) {
            *error = "region_id_missing";
            return false;
        }
        if (!isNonZeroU64(region.startAddress)) {
            *error = "region_start_address_invalid";
            return false;
        }
        if (!isNonZeroU64(region.sizeBytes)) {
            *error = "region_size_invalid";
            return false;
        }
        if (findRegionIndexById(*snapshot, region.id) != -1) {
            *error = "region_id_duplicate";
            return false;
        }

        snapshot->regions.push_back(region);
        stableSortRegions(snapshot);
        return true;
    }

    static bool addReference(MemorySnapshot* snapshot,
                             const MemoryReferenceRecord& reference,
                             std::string* error) {
        if (!snapshot || !error) return false;
        error->clear();
        if (!isNonZeroU64(reference.fromAddress) || !isNonZeroU64(reference.toAddress)) {
            *error = "reference_address_invalid";
            return false;
        }
        if (reference.label.empty()) {
            *error = "reference_label_missing";
            return false;
        }

        snapshot->references.push_back(reference);
        return true;
    }

    static const MemoryRegionRecord* findRegionByAddress(const MemorySnapshot& snapshot,
                                                         std::uint64_t address) {
        if (!isNonZeroU64(address)) return nullptr;
        for (const auto& region : snapshot.regions) {
            const std::uint64_t regionEnd = region.startAddress + region.sizeBytes;
            if (address >= region.startAddress && address < regionEnd) return &region;
        }
        return nullptr;
    }

    static std::vector<MemoryReferenceRecord> outgoingReferences(const MemorySnapshot& snapshot,
                                                                 std::uint64_t fromAddress) {
        std::vector<MemoryReferenceRecord> filtered;
        if (!isNonZeroU64(fromAddress)) return filtered;
        for (const auto& reference : snapshot.references) {
            if (reference.fromAddress == fromAddress) filtered.push_back(reference);
        }
        return filtered;
    }

private:
    static int findRegionIndexById(const MemorySnapshot& snapshot, const std::string& id) {
        for (size_t i = 0; i < snapshot.regions.size(); ++i) {
            if (snapshot.regions[i].id == id) return static_cast<int>(i);
        }
        return -1;
    }

    static void stableSortRegions(MemorySnapshot* snapshot) {
        std::stable_sort(snapshot->regions.begin(),
                         snapshot->regions.end(),
                         [](const MemoryRegionRecord& a, const MemoryRegionRecord& b) {
                             if (a.startAddress != b.startAddress) return a.startAddress < b.startAddress;
                             return a.id < b.id;
                         });
    }
};
