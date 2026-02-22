#pragma once
// Step 825: Feedback-to-adapter tuning hooks.

#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct FeedbackSignal {
    std::string signalId;
    std::string sourceIssue;
    std::string adapterTarget;
    std::string suggestion;
    int weight = 1;  // 1-10
};

struct AdapterTuningHint {
    std::string adapterTarget;
    std::vector<FeedbackSignal> signals;
    int aggregateWeight = 0;
    std::string topSuggestion;
};

class FeedbackAdapterHooks {
public:
    bool addSignal(const FeedbackSignal& sig, std::string* error) {
        if (!error) return false;
        error->clear();
        if (sig.signalId.empty())      { *error = "signal_id_missing";     return false; }
        if (sig.adapterTarget.empty()) { *error = "adapter_target_missing"; return false; }
        if (sig.suggestion.empty())    { *error = "suggestion_missing";     return false; }
        signals_.push_back(sig);
        return true;
    }

    AdapterTuningHint buildHint(const std::string& adapterTarget) const {
        AdapterTuningHint hint;
        hint.adapterTarget = adapterTarget;
        int maxW = 0;
        for (const auto& s : signals_) {
            if (s.adapterTarget == adapterTarget) {
                hint.signals.push_back(s);
                hint.aggregateWeight += s.weight;
                if (s.weight > maxW) {
                    maxW = s.weight;
                    hint.topSuggestion = s.suggestion;
                }
            }
        }
        return hint;
    }

    std::vector<std::string> adapterTargets() const {
        std::vector<std::string> out;
        for (const auto& s : signals_) {
            bool found = false;
            for (const auto& t : out) if (t == s.adapterTarget) { found = true; break; }
            if (!found) out.push_back(s.adapterTarget);
        }
        return out;
    }

    static nlohmann::json toJson(const AdapterTuningHint& h) {
        nlohmann::json sigs = nlohmann::json::array();
        for (const auto& s : h.signals) sigs.push_back({{"signal_id", s.signalId}, {"suggestion", s.suggestion}, {"weight", s.weight}});
        return {{"adapter_target", h.adapterTarget}, {"signals", sigs},
                {"aggregate_weight", h.aggregateWeight}, {"top_suggestion", h.topSuggestion}};
    }

private:
    std::vector<FeedbackSignal> signals_;
};
