#pragma once
// Step 566: Allocation/Ownership Trace Hooks

#include "DebugValidationUtil.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

enum class AllocationTraceEventType {
    Alloc,
    Transfer,
    Free
};

struct AllocationTraceEvent {
    std::string eventId;
    std::string sessionId;
    std::string allocationId;
    AllocationTraceEventType type = AllocationTraceEventType::Alloc;
    std::uint64_t timestamp = 0;
    std::uint64_t address = 0;
    std::uint64_t sizeBytes = 0;
    std::string ownerFrom;
    std::string ownerTo;
};

class AllocationOwnershipTraceHooks {
public:
    bool recordAllocation(const std::string& sessionId,
                          const std::string& allocationId,
                          std::uint64_t address,
                          std::uint64_t sizeBytes,
                          const std::string& owner,
                          std::uint64_t timestamp,
                          std::string* error) {
        if (!error) return false;
        error->clear();
        if (!hasRequiredDebugIds(allocationId, sessionId)) return fail(error, "allocation_or_session_missing");
        if (!isNonZeroU64(address)) return fail(error, "allocation_address_invalid");
        if (!isNonZeroU64(sizeBytes)) return fail(error, "allocation_size_invalid");
        if (owner.empty()) return fail(error, "allocation_owner_missing");
        if (activeOwners_.count(allocationId) != 0) return fail(error, "allocation_id_active_duplicate");

        activeOwners_[allocationId] = owner;
        AllocationTraceEvent event;
        event.eventId = nextEventId();
        event.sessionId = sessionId;
        event.allocationId = allocationId;
        event.type = AllocationTraceEventType::Alloc;
        event.timestamp = timestamp;
        event.address = address;
        event.sizeBytes = sizeBytes;
        event.ownerTo = owner;
        appendEvent(event);
        return true;
    }

    bool transferOwnership(const std::string& sessionId,
                           const std::string& allocationId,
                           const std::string& newOwner,
                           std::uint64_t timestamp,
                           std::string* error) {
        if (!error) return false;
        error->clear();
        if (!hasRequiredDebugIds(allocationId, sessionId)) return fail(error, "allocation_or_session_missing");
        if (newOwner.empty()) return fail(error, "transfer_owner_missing");

        auto it = activeOwners_.find(allocationId);
        if (it == activeOwners_.end()) return fail(error, "allocation_not_active");
        if (it->second == newOwner) return fail(error, "transfer_owner_unchanged");

        AllocationTraceEvent event;
        event.eventId = nextEventId();
        event.sessionId = sessionId;
        event.allocationId = allocationId;
        event.type = AllocationTraceEventType::Transfer;
        event.timestamp = timestamp;
        event.ownerFrom = it->second;
        event.ownerTo = newOwner;
        it->second = newOwner;
        appendEvent(event);
        return true;
    }

    bool recordFree(const std::string& sessionId,
                    const std::string& allocationId,
                    std::uint64_t timestamp,
                    std::string* error) {
        if (!error) return false;
        error->clear();
        if (!hasRequiredDebugIds(allocationId, sessionId)) return fail(error, "allocation_or_session_missing");

        auto it = activeOwners_.find(allocationId);
        if (it == activeOwners_.end()) return fail(error, "allocation_not_active");

        AllocationTraceEvent event;
        event.eventId = nextEventId();
        event.sessionId = sessionId;
        event.allocationId = allocationId;
        event.type = AllocationTraceEventType::Free;
        event.timestamp = timestamp;
        event.ownerFrom = it->second;
        activeOwners_.erase(it);
        appendEvent(event);
        return true;
    }

    std::string ownerFor(const std::string& allocationId) const {
        auto it = activeOwners_.find(allocationId);
        if (it == activeOwners_.end()) return "";
        return it->second;
    }

    std::size_t activeAllocationCount() const {
        return activeOwners_.size();
    }

    std::vector<AllocationTraceEvent> timelineForSession(const std::string& sessionId) const {
        std::vector<AllocationTraceEvent> filtered;
        for (const auto& event : events_) {
            if (sessionId.empty() || event.sessionId == sessionId) filtered.push_back(event);
        }
        return filtered;
    }

private:
    std::vector<AllocationTraceEvent> events_;
    std::map<std::string, std::string> activeOwners_;
    std::uint64_t nextId_ = 1;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    std::string nextEventId() {
        return "alloc-event-" + std::to_string(nextId_++);
    }

    void appendEvent(const AllocationTraceEvent& event) {
        events_.push_back(event);
        std::stable_sort(events_.begin(), events_.end(), [](const AllocationTraceEvent& a, const AllocationTraceEvent& b) {
            if (a.timestamp != b.timestamp) return a.timestamp < b.timestamp;
            return a.eventId < b.eventId;
        });
    }
};
