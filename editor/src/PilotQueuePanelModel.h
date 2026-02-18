#pragma once
// Step 661: Pilot queue panel model

#include <string>
#include <vector>

enum class PilotDecision { Pending, Approved, Rejected, Modified };

struct PilotQueueItem {
    std::string jobId;
    std::string summary;
    bool escalate = false;
    double ambiguity = 0.0;
    PilotDecision decision = PilotDecision::Pending;
    std::string strategyNote;
};

class PilotQueuePanelModel {
public:
    static bool needsHumanReview(const PilotQueueItem& item, double threshold = 0.5) {
        return item.escalate || item.ambiguity > threshold;
    }

    static void approve(PilotQueueItem* item) {
        if (!item) return;
        item->decision = PilotDecision::Approved;
    }

    static void reject(PilotQueueItem* item) {
        if (!item) return;
        item->decision = PilotDecision::Rejected;
    }

    static void modify(PilotQueueItem* item, const std::string& note) {
        if (!item) return;
        item->decision = PilotDecision::Modified;
        item->strategyNote = note;
    }
};
