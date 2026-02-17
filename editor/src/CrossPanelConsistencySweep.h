#pragma once
// Step 522: Cross-Panel Consistency Sweep

#include <string>
#include <vector>
#include <map>
#include <algorithm>

struct PanelVisualContract {
    std::string panelId;
    float spacing = 0.0f;
    float elevation = 0.0f;
    float border = 0.0f;
    std::string stateModelVersion;
    std::string tokenVersion;
};

struct ConsistencyReport {
    bool pass = true;
    std::vector<std::string> drifts;
    int checkedPanels = 0;
};

class CrossPanelConsistencySweep {
public:
    static ConsistencyReport run(const std::vector<PanelVisualContract>& panels,
                                 const PanelVisualContract& baseline,
                                 float spacingTolerance = 1.0f,
                                 float elevationTolerance = 1.0f,
                                 float borderTolerance = 0.5f) {
        ConsistencyReport r;
        r.checkedPanels = (int)panels.size();
        for (const auto& p : panels) {
            if (std::abs(p.spacing - baseline.spacing) > spacingTolerance) {
                r.pass = false;
                r.drifts.push_back("spacing:" + p.panelId);
            }
            if (std::abs(p.elevation - baseline.elevation) > elevationTolerance) {
                r.pass = false;
                r.drifts.push_back("elevation:" + p.panelId);
            }
            if (std::abs(p.border - baseline.border) > borderTolerance) {
                r.pass = false;
                r.drifts.push_back("border:" + p.panelId);
            }
            if (p.stateModelVersion != baseline.stateModelVersion) {
                r.pass = false;
                r.drifts.push_back("state-model:" + p.panelId);
            }
            if (p.tokenVersion != baseline.tokenVersion) {
                r.pass = false;
                r.drifts.push_back("token-version:" + p.panelId);
            }
        }
        return r;
    }

    static std::vector<PanelVisualContract> normalize(
            const std::vector<PanelVisualContract>& panels,
            const PanelVisualContract& baseline) {
        std::vector<PanelVisualContract> out = panels;
        for (auto& p : out) {
            p.spacing = baseline.spacing;
            p.elevation = baseline.elevation;
            p.border = baseline.border;
            p.stateModelVersion = baseline.stateModelVersion;
            p.tokenVersion = baseline.tokenVersion;
        }
        return out;
    }

    static std::map<std::string, int> driftHistogram(const ConsistencyReport& r) {
        std::map<std::string, int> h;
        for (const auto& d : r.drifts) {
            auto pos = d.find(':');
            std::string key = pos == std::string::npos ? d : d.substr(0, pos);
            h[key] += 1;
        }
        return h;
    }
};
