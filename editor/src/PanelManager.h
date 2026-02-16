#pragma once
// Step 342-343: Panel registration system and docking layout management
// Headless data model for panel state — no ImGui dependency

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <nlohmann/json.hpp>

enum class DockPosition {
    Left,
    CenterTop,
    CenterBottom,
    Right,
    Bottom
};

inline std::string dockPositionToString(DockPosition p) {
    switch (p) {
        case DockPosition::Left: return "left";
        case DockPosition::CenterTop: return "center-top";
        case DockPosition::CenterBottom: return "center-bottom";
        case DockPosition::Right: return "right";
        case DockPosition::Bottom: return "bottom";
    }
    return "unknown";
}

inline DockPosition dockPositionFromString(const std::string& s) {
    if (s == "left") return DockPosition::Left;
    if (s == "center-top") return DockPosition::CenterTop;
    if (s == "center-bottom") return DockPosition::CenterBottom;
    if (s == "right") return DockPosition::Right;
    if (s == "bottom") return DockPosition::Bottom;
    return DockPosition::CenterTop;
}

struct PanelInfo {
    std::string id;
    std::string title;
    DockPosition defaultDock;
    DockPosition currentDock;
    bool isVisible = true;
    int order = 0;
    std::string icon;         // Unicode icon character
    std::string category;     // "navigation", "editor", "analysis", "output"
    float widthPercent = 0;   // 0 = auto
    float heightPercent = 0;  // 0 = auto

    nlohmann::json toJson() const {
        return {
            {"id", id}, {"title", title},
            {"defaultDock", dockPositionToString(defaultDock)},
            {"currentDock", dockPositionToString(currentDock)},
            {"isVisible", isVisible}, {"order", order},
            {"icon", icon}, {"category", category},
            {"widthPercent", widthPercent}, {"heightPercent", heightPercent}
        };
    }

    static PanelInfo fromJson(const nlohmann::json& j) {
        PanelInfo p;
        p.id = j.value("id", "");
        p.title = j.value("title", "");
        p.defaultDock = dockPositionFromString(j.value("defaultDock", "center-top"));
        p.currentDock = dockPositionFromString(j.value("currentDock", j.value("defaultDock", "center-top")));
        p.isVisible = j.value("isVisible", true);
        p.order = j.value("order", 0);
        p.icon = j.value("icon", "");
        p.category = j.value("category", "");
        p.widthPercent = j.value("widthPercent", 0.0f);
        p.heightPercent = j.value("heightPercent", 0.0f);
        return p;
    }
};

struct DockingLayout {
    float leftWidth = 20.0f;      // percent
    float rightWidth = 20.0f;     // percent
    float bottomHeight = 25.0f;   // percent
    float centerWidth = 60.0f;    // auto-computed

    nlohmann::json toJson() const {
        return {
            {"leftWidth", leftWidth}, {"rightWidth", rightWidth},
            {"bottomHeight", bottomHeight}
        };
    }

    static DockingLayout fromJson(const nlohmann::json& j) {
        DockingLayout l;
        l.leftWidth = j.value("leftWidth", 20.0f);
        l.rightWidth = j.value("rightWidth", 20.0f);
        l.bottomHeight = j.value("bottomHeight", 25.0f);
        l.centerWidth = 100.0f - l.leftWidth - l.rightWidth;
        return l;
    }
};

class PanelManager {
public:
    PanelManager() = default;

    void registerPanel(const std::string& id, const std::string& title,
                       DockPosition dock, const std::string& icon = "",
                       const std::string& category = "", int order = 0) {
        PanelInfo info;
        info.id = id;
        info.title = title;
        info.defaultDock = dock;
        info.currentDock = dock;
        info.icon = icon;
        info.category = category;
        info.order = order;
        panels_[id] = info;
    }

    void setVisible(const std::string& id, bool visible) {
        auto it = panels_.find(id);
        if (it != panels_.end()) it->second.isVisible = visible;
    }

    bool isVisible(const std::string& id) const {
        auto it = panels_.find(id);
        return it != panels_.end() && it->second.isVisible;
    }

    void togglePanel(const std::string& id) {
        auto it = panels_.find(id);
        if (it != panels_.end()) it->second.isVisible = !it->second.isVisible;
    }

    void focusPanel(const std::string& id) {
        focusedPanel_ = id;
    }

    std::string getFocusedPanel() const { return focusedPanel_; }

    bool hasPanel(const std::string& id) const {
        return panels_.count(id) > 0;
    }

    PanelInfo getPanel(const std::string& id) const {
        auto it = panels_.find(id);
        if (it != panels_.end()) return it->second;
        return {};
    }

    std::vector<PanelInfo> getPanelList() const {
        std::vector<PanelInfo> result;
        for (const auto& [id, info] : panels_) result.push_back(info);
        std::sort(result.begin(), result.end(),
                  [](const PanelInfo& a, const PanelInfo& b) { return a.order < b.order; });
        return result;
    }

    std::vector<PanelInfo> getVisiblePanels() const {
        std::vector<PanelInfo> result;
        for (const auto& [id, info] : panels_) {
            if (info.isVisible) result.push_back(info);
        }
        std::sort(result.begin(), result.end(),
                  [](const PanelInfo& a, const PanelInfo& b) { return a.order < b.order; });
        return result;
    }

    std::vector<PanelInfo> getPanelsByDock(DockPosition dock) const {
        std::vector<PanelInfo> result;
        for (const auto& [id, info] : panels_) {
            if (info.currentDock == dock && info.isVisible) result.push_back(info);
        }
        std::sort(result.begin(), result.end(),
                  [](const PanelInfo& a, const PanelInfo& b) { return a.order < b.order; });
        return result;
    }

    void movePanelToDock(const std::string& id, DockPosition dock) {
        auto it = panels_.find(id);
        if (it != panels_.end()) it->second.currentDock = dock;
    }

    int panelCount() const { return static_cast<int>(panels_.size()); }

    void registerDefaults() {
        registerPanel("file-tree", "File Tree", DockPosition::Left, "\xF0\x9F\x93\x81", "navigation", 0);
        registerPanel("code-editor", "Code Editor", DockPosition::CenterTop, "\xE2\x9C\x8F", "editor", 1);
        registerPanel("ast-view", "AST View", DockPosition::Right, "\xF0\x9F\x8C\xB3", "analysis", 2);
        registerPanel("annotations", "Annotations", DockPosition::Right, "\xF0\x9F\x8F\xB7", "analysis", 3);
        registerPanel("properties", "Properties", DockPosition::Right, "\xE2\x9A\x99", "analysis", 4);
        registerPanel("diagnostics", "Diagnostics", DockPosition::Bottom, "\xE2\x9A\xA0", "output", 5);
        registerPanel("output", "Output", DockPosition::Bottom, "\xF0\x9F\x93\x84", "output", 6);
        registerPanel("workflow", "Workflow", DockPosition::Bottom, "\xF0\x9F\x94\x84", "output", 7);
    }

    // Layout management
    DockingLayout& layout() { return layout_; }
    const DockingLayout& layout() const { return layout_; }

    nlohmann::json toJson() const {
        nlohmann::json j;
        j["layout"] = layout_.toJson();
        nlohmann::json panelsJson = nlohmann::json::array();
        for (const auto& [id, info] : panels_) {
            panelsJson.push_back(info.toJson());
        }
        j["panels"] = panelsJson;
        j["focusedPanel"] = focusedPanel_;
        return j;
    }

    static PanelManager fromJson(const nlohmann::json& j) {
        PanelManager pm;
        if (j.contains("layout")) pm.layout_ = DockingLayout::fromJson(j["layout"]);
        if (j.contains("panels")) {
            for (const auto& pj : j["panels"]) {
                PanelInfo info = PanelInfo::fromJson(pj);
                pm.panels_[info.id] = info;
            }
        }
        pm.focusedPanel_ = j.value("focusedPanel", "");
        return pm;
    }

private:
    std::map<std::string, PanelInfo> panels_;
    DockingLayout layout_;
    std::string focusedPanel_;
};
