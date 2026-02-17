// -----------------------------------------------------------------------
//  Step 252: DiagnosticVersionTracker — track diagnostic changes
// -----------------------------------------------------------------------

// Key that identifies a unique diagnostic instance
struct DiagnosticKey {
    std::string code;
    std::string nodeId;
    int         line = 0;

    bool operator==(const DiagnosticKey& o) const {
        return code == o.code && nodeId == o.nodeId && line == o.line;
    }
    bool operator<(const DiagnosticKey& o) const {
        if (code != o.code) return code < o.code;
        if (nodeId != o.nodeId) return nodeId < o.nodeId;
        return line < o.line;
    }
};

inline DiagnosticKey keyFromDiagnostic(const StructuredDiagnostic& d) {
    return {d.code, d.nodeId, d.line};
}

struct DiagnosticDelta {
    std::vector<StructuredDiagnostic> added;
    std::vector<StructuredDiagnostic> removed;
    int version = 0;
};

class DiagnosticVersionTracker {
public:
    int version = 0;

    // Record current diagnostics snapshot and bump version
    void recordSnapshot(const std::vector<StructuredDiagnostic>& diags) {
        previousDiags_ = currentDiags_;
        previousVersion_ = version;
        currentDiags_ = diags;
        ++version;
    }

    // Compute delta: what changed since the given version
    DiagnosticDelta getDelta(
            const std::vector<StructuredDiagnostic>& currentDiags,
            int sinceVersion) const {
        DiagnosticDelta delta;
        delta.version = version;

        if (sinceVersion >= version) {
            // No changes since that version
            return delta;
        }

        // Build key sets for previous and current
        std::set<DiagnosticKey> prevKeys;
        std::map<DiagnosticKey, StructuredDiagnostic> prevMap;
        for (const auto& d : previousDiags_) {
            auto key = keyFromDiagnostic(d);
            prevKeys.insert(key);
            prevMap[key] = d;
        }

        std::set<DiagnosticKey> currKeys;
        std::map<DiagnosticKey, StructuredDiagnostic> currMap;
        for (const auto& d : currentDiags) {
            auto key = keyFromDiagnostic(d);
            currKeys.insert(key);
            currMap[key] = d;
        }

        // Added: in current but not in previous
        for (const auto& key : currKeys) {
            if (prevKeys.find(key) == prevKeys.end()) {
                delta.added.push_back(currMap[key]);
            }
        }

        // Removed: in previous but not in current
        for (const auto& key : prevKeys) {
            if (currKeys.find(key) == currKeys.end()) {
                delta.removed.push_back(prevMap[key]);
            }
        }

        return delta;
    }

    // Reset tracker
    void reset() {
        version = 0;
        previousVersion_ = 0;
        currentDiags_.clear();
        previousDiags_.clear();
    }

private:
    int previousVersion_ = 0;
    std::vector<StructuredDiagnostic> currentDiags_;
    std::vector<StructuredDiagnostic> previousDiags_;
};

inline json deltaToJson(const DiagnosticDelta& delta) {
    return {
        {"added",    diagnosticsToJson(delta.added)},
        {"removed",  diagnosticsToJson(delta.removed)},
        {"addedCount",   (int)delta.added.size()},
        {"removedCount", (int)delta.removed.size()},
        {"version",  delta.version}
    };
}
