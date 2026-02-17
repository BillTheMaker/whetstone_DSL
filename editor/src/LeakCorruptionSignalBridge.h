#pragma once
// Step 567: Leak/Corruption Signal Bridge

#include "DebugValidationUtil.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

enum class MemorySignalType {
    LeakSuspected,
    LeakConfirmed,
    CorruptionSuspected,
    CorruptionConfirmed
};

enum class MemorySignalSeverity {
    Info = 0,
    Warning = 1,
    Error = 2,
    Critical = 3
};

struct MemorySignalRecord {
    std::string signalId;
    std::string sessionId;
    std::string allocationId;
    MemorySignalType type = MemorySignalType::LeakSuspected;
    std::string message;
    std::uint64_t timestamp = 0;
};

struct MemoryDiagnosticRecord {
    std::string diagnosticId;
    std::string sessionId;
    std::string allocationId;
    std::string paneKey;
    std::string message;
    MemorySignalSeverity severity = MemorySignalSeverity::Info;
    std::uint64_t timestamp = 0;
};

class LeakCorruptionSignalBridge {
public:
    bool ingestSignal(const MemorySignalRecord& signal, std::string* error) {
        if (!error) return false;
        error->clear();
        if (!hasRequiredDebugIds(signal.signalId, signal.sessionId)) return fail(error, "signal_or_session_missing");
        if (!hasRequiredDebugIds(signal.signalId, signal.sessionId, signal.allocationId)) return fail(error, "allocation_id_missing");
        if (signal.message.empty()) return fail(error, "signal_message_missing");
        if (!isNonZeroU64(signal.timestamp)) return fail(error, "signal_timestamp_invalid");
        if (containsSignal(signal.signalId)) return fail(error, "signal_duplicate");

        signals_.push_back(signal);
        diagnostics_.push_back(buildDiagnostic(signal));
        stableSortByTimestamp(&signals_);
        stableSortByTimestamp(&diagnostics_);
        return true;
    }

    std::vector<MemoryDiagnosticRecord> diagnosticsForSession(const std::string& sessionId) const {
        std::vector<MemoryDiagnosticRecord> filtered;
        for (const auto& diagnostic : diagnostics_) {
            if (sessionId.empty() || diagnostic.sessionId == sessionId) filtered.push_back(diagnostic);
        }
        return filtered;
    }

    std::vector<MemorySignalRecord> signalsForAllocation(const std::string& allocationId) const {
        std::vector<MemorySignalRecord> filtered;
        for (const auto& signal : signals_) {
            if (allocationId.empty() || signal.allocationId == allocationId) filtered.push_back(signal);
        }
        return filtered;
    }

    MemorySignalSeverity highestSeverityForSession(const std::string& sessionId) const {
        MemorySignalSeverity highest = MemorySignalSeverity::Info;
        for (const auto& diagnostic : diagnostics_) {
            if (!sessionId.empty() && diagnostic.sessionId != sessionId) continue;
            if (static_cast<int>(diagnostic.severity) > static_cast<int>(highest)) highest = diagnostic.severity;
        }
        return highest;
    }

private:
    std::vector<MemorySignalRecord> signals_;
    std::vector<MemoryDiagnosticRecord> diagnostics_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    bool containsSignal(const std::string& signalId) const {
        for (const auto& signal : signals_) {
            if (signal.signalId == signalId) return true;
        }
        return false;
    }

    static MemoryDiagnosticRecord buildDiagnostic(const MemorySignalRecord& signal) {
        MemoryDiagnosticRecord diagnostic;
        diagnostic.diagnosticId = "diag-" + signal.signalId;
        diagnostic.sessionId = signal.sessionId;
        diagnostic.allocationId = signal.allocationId;
        diagnostic.message = signal.message;
        diagnostic.timestamp = signal.timestamp;
        diagnostic.paneKey = paneForSignal(signal.type);
        diagnostic.severity = severityForSignal(signal.type);
        return diagnostic;
    }

    static std::string paneForSignal(MemorySignalType type) {
        switch (type) {
            case MemorySignalType::LeakSuspected:
            case MemorySignalType::LeakConfirmed:
                return "memory-leaks";
            case MemorySignalType::CorruptionSuspected:
            case MemorySignalType::CorruptionConfirmed:
                return "memory-corruption";
        }
        return "memory-diagnostics";
    }

    static MemorySignalSeverity severityForSignal(MemorySignalType type) {
        switch (type) {
            case MemorySignalType::LeakSuspected: return MemorySignalSeverity::Warning;
            case MemorySignalType::LeakConfirmed: return MemorySignalSeverity::Error;
            case MemorySignalType::CorruptionSuspected: return MemorySignalSeverity::Error;
            case MemorySignalType::CorruptionConfirmed: return MemorySignalSeverity::Critical;
        }
        return MemorySignalSeverity::Info;
    }

    static void stableSortByTimestamp(std::vector<MemorySignalRecord>* signals) {
        std::stable_sort(signals->begin(),
                         signals->end(),
                         [](const MemorySignalRecord& a, const MemorySignalRecord& b) {
                             if (a.timestamp != b.timestamp) return a.timestamp < b.timestamp;
                             return a.signalId < b.signalId;
                         });
    }

    static void stableSortByTimestamp(std::vector<MemoryDiagnosticRecord>* diagnostics) {
        std::stable_sort(diagnostics->begin(),
                         diagnostics->end(),
                         [](const MemoryDiagnosticRecord& a, const MemoryDiagnosticRecord& b) {
                             if (a.timestamp != b.timestamp) return a.timestamp < b.timestamp;
                             return a.diagnosticId < b.diagnosticId;
                         });
    }
};
