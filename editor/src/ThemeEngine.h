#pragma once
// Step 160: Theme engine

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include "imgui.h"
#include "SyntaxHighlighter.h"

using json = nlohmann::json;

enum class ThemeColor {
    EditorBg,
    EditorFg,
    EditorCursor,
    EditorSelection,
    EditorCurrentLine,
    SyntaxKeyword,
    SyntaxString,
    SyntaxComment,
    SyntaxNumber,
    SyntaxType,
    SyntaxFunction,
    SyntaxOperator,
    GutterBg,
    GutterText,
    GutterMarkerError,
    GutterMarkerWarning,
    PanelBg,
    PanelBorder,
    PanelHeader,
    StatusBarBg,
    TooltipBg
};

struct ThemeDefinition {
    std::string name;
    std::unordered_map<std::string, ImU32> syntaxColors;
    std::unordered_map<std::string, ImU32> editorColors;
    std::unordered_map<int, ImVec4> imguiColors;
    std::string sourcePath;
};

class ThemeEngine {
public:
    static ThemeEngine& instance() {
        static ThemeEngine inst;
        return inst;
    }

    void clear() {
        themes_.clear();
        current_ = -1;
    }

    bool loadThemesFromDirectory(const std::string& dir, bool watch = true) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!fs::exists(dir, ec)) return false;
        if (watch) {
            bool tracked = false;
            for (const auto& existing : watchedDirs_) {
                if (existing == dir) {
                    tracked = true;
                    break;
                }
            }
            if (!tracked) watchedDirs_.push_back(dir);
        }
        bool loaded = false;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;
            auto path = entry.path();
            if (path.extension() != ".json") continue;
            if (loadThemeFile(path.string())) loaded = true;
        }
        return loaded;
    }

    bool loadThemeFile(const std::string& path) {
        std::ifstream in(path);
        if (!in.is_open()) return false;
        std::ostringstream ss;
        ss << in.rdbuf();
        if (!loadThemeFromJson(ss.str())) return false;
        fileTimes_[path] = std::filesystem::last_write_time(path);
        return true;
    }

    bool loadThemeFromJson(const std::string& text) {
        try {
            json j = json::parse(text);
            ThemeDefinition theme;
            if (!parseTheme(j, theme)) return false;
            registerTheme(theme);
            return true;
        } catch (...) {
            return false;
        }
    }

    void registerTheme(const ThemeDefinition& theme) {
        for (auto& t : themes_) {
            if (t.name == theme.name) {
                t = theme;
                return;
            }
        }
        themes_.push_back(theme);
    }

    std::vector<std::string> listThemes() const {
        std::vector<std::string> names;
        for (const auto& t : themes_) names.push_back(t.name);
        std::sort(names.begin(), names.end());
        return names;
    }

    bool applyTheme(const std::string& name) {
        for (size_t i = 0; i < themes_.size(); ++i) {
            if (themes_[i].name == name) {
                current_ = (int)i;
                applyToImGui(themes_[i]);
                return true;
            }
        }
        return false;
    }

    ImU32 getColor(ThemeColor color, ImU32 fallback) const {
        const ThemeDefinition* theme = currentTheme();
        if (!theme) return fallback;
        const std::string key = colorKey(color);
        if (key.empty()) return fallback;
        if (isSyntaxColor(color)) {
            auto it = theme->syntaxColors.find(key);
            return it != theme->syntaxColors.end() ? it->second : fallback;
        }
        auto it = theme->editorColors.find(key);
        return it != theme->editorColors.end() ? it->second : fallback;
    }

    std::string currentThemeName() const {
        if (current_ < 0 || current_ >= (int)themes_.size()) return "";
        return themes_[current_].name;
    }

    ImU32 syntaxColor(TokenCategory cat, ImU32 fallback) const {
        const ThemeDefinition* theme = currentTheme();
        if (!theme) return fallback;
        std::string key = SyntaxHighlighter::categoryName(cat);
        auto it = theme->syntaxColors.find(key);
        return it != theme->syntaxColors.end() ? it->second : fallback;
    }

    ImU32 editorColor(const std::string& key, ImU32 fallback) const {
        const ThemeDefinition* theme = currentTheme();
        if (!theme) return fallback;
        auto it = theme->editorColors.find(key);
        return it != theme->editorColors.end() ? it->second : fallback;
    }

    bool refreshWatchedThemes() {
        namespace fs = std::filesystem;
        bool reloaded = false;
        for (const auto& dir : watchedDirs_) {
            std::error_code ec;
            if (!fs::exists(dir, ec)) continue;
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (!entry.is_regular_file()) continue;
                auto path = entry.path();
                if (path.extension() != ".json") continue;
                auto pathStr = path.string();
                auto ts = fs::last_write_time(path, ec);
                if (ec) continue;
                auto it = fileTimes_.find(pathStr);
                if (it == fileTimes_.end() || it->second != ts) {
                    if (loadThemeFile(pathStr)) {
                        reloaded = true;
                    }
                }
            }
        }
        if (reloaded && current_ >= 0 && current_ < (int)themes_.size()) {
            applyToImGui(themes_[current_]);
        }
        return reloaded;
    }

    static std::string userThemeDirectory() {
        const char* home = std::getenv("USERPROFILE");
        if (!home) home = std::getenv("HOME");
        std::filesystem::path base = home ? std::filesystem::path(home)
                                          : std::filesystem::current_path();
        return (base / ".whetstone" / "themes").string();
    }

private:
    ThemeEngine() = default;

    const ThemeDefinition* currentTheme() const {
        if (current_ < 0 || current_ >= (int)themes_.size()) return nullptr;
        return &themes_[current_];
    }

    static bool parseHexColor(const std::string& text,
                              ImU32& outU32,
                              ImVec4& outVec) {
        std::string hex = text;
        if (!hex.empty() && hex[0] == '#') hex = hex.substr(1);
        if (hex.size() != 6 && hex.size() != 8) return false;
        auto toByte = [](const std::string& s) {
            return (int)std::strtol(s.c_str(), nullptr, 16);
        };
        int r = toByte(hex.substr(0, 2));
        int g = toByte(hex.substr(2, 2));
        int b = toByte(hex.substr(4, 2));
        int a = 255;
        if (hex.size() == 8) a = toByte(hex.substr(6, 2));
        outU32 = IM_COL32(r, g, b, a);
        outVec = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        return true;
    }

    static int imguiColorId(const std::string& name) {
        if (name == "Text") return ImGuiCol_Text;
        if (name == "WindowBg") return ImGuiCol_WindowBg;
        if (name == "ChildBg") return ImGuiCol_ChildBg;
        if (name == "PopupBg") return ImGuiCol_PopupBg;
        if (name == "Border") return ImGuiCol_Border;
        if (name == "FrameBg") return ImGuiCol_FrameBg;
        if (name == "FrameBgHovered") return ImGuiCol_FrameBgHovered;
        if (name == "FrameBgActive") return ImGuiCol_FrameBgActive;
        if (name == "TitleBg") return ImGuiCol_TitleBg;
        if (name == "TitleBgActive") return ImGuiCol_TitleBgActive;
        if (name == "Header") return ImGuiCol_Header;
        if (name == "HeaderHovered") return ImGuiCol_HeaderHovered;
        if (name == "HeaderActive") return ImGuiCol_HeaderActive;
        if (name == "Tab") return ImGuiCol_Tab;
        if (name == "TabHovered") return ImGuiCol_TabHovered;
        if (name == "TabActive") return ImGuiCol_TabActive;
        if (name == "TabUnfocused") return ImGuiCol_TabUnfocused;
        if (name == "TabUnfocusedActive") return ImGuiCol_TabUnfocusedActive;
        if (name == "Button") return ImGuiCol_Button;
        if (name == "ButtonHovered") return ImGuiCol_ButtonHovered;
        if (name == "ButtonActive") return ImGuiCol_ButtonActive;
        if (name == "ScrollbarBg") return ImGuiCol_ScrollbarBg;
        if (name == "ScrollbarGrab") return ImGuiCol_ScrollbarGrab;
        if (name == "ScrollbarGrabHovered") return ImGuiCol_ScrollbarGrabHovered;
        if (name == "ScrollbarGrabActive") return ImGuiCol_ScrollbarGrabActive;
        if (name == "CheckMark") return ImGuiCol_CheckMark;
        if (name == "SliderGrab") return ImGuiCol_SliderGrab;
        if (name == "SliderGrabActive") return ImGuiCol_SliderGrabActive;
        if (name == "Separator") return ImGuiCol_Separator;
        if (name == "SeparatorHovered") return ImGuiCol_SeparatorHovered;
        if (name == "SeparatorActive") return ImGuiCol_SeparatorActive;
        return -1;
    }

    static bool parseTheme(const json& j, ThemeDefinition& out) {
        if (!j.is_object()) return false;
        out.name = j.value("name", "");
        if (out.name.empty()) return false;

        if (j.contains("syntax") && j["syntax"].is_object()) {
            for (const auto& [key, value] : j["syntax"].items()) {
                if (!value.is_string()) continue;
                ImU32 u32;
                ImVec4 v4;
                if (parseHexColor(value.get<std::string>(), u32, v4)) {
                    out.syntaxColors[key] = u32;
                }
            }
        }

        if (j.contains("editor") && j["editor"].is_object()) {
            for (const auto& [key, value] : j["editor"].items()) {
                if (!value.is_string()) continue;
                ImU32 u32;
                ImVec4 v4;
                if (parseHexColor(value.get<std::string>(), u32, v4)) {
                    out.editorColors[key] = u32;
                }
            }
        }

        if (j.contains("imgui") && j["imgui"].is_object()) {
            for (const auto& [key, value] : j["imgui"].items()) {
                if (!value.is_string()) continue;
                ImU32 u32;
                ImVec4 v4;
                if (!parseHexColor(value.get<std::string>(), u32, v4)) continue;
                int colId = imguiColorId(key);
                if (colId >= 0) {
                    out.imguiColors[colId] = v4;
                }
            }
        }
        return true;
    }

    static void applyToImGui(const ThemeDefinition& theme) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (const auto& [colId, color] : theme.imguiColors) {
            if (colId >= 0 && colId < ImGuiCol_COUNT) {
                style.Colors[colId] = color;
            }
        }
    }

    static bool isSyntaxColor(ThemeColor color) {
        return color == ThemeColor::SyntaxKeyword ||
               color == ThemeColor::SyntaxString ||
               color == ThemeColor::SyntaxComment ||
               color == ThemeColor::SyntaxNumber ||
               color == ThemeColor::SyntaxType ||
               color == ThemeColor::SyntaxFunction ||
               color == ThemeColor::SyntaxOperator;
    }

    static std::string colorKey(ThemeColor color) {
        switch (color) {
        case ThemeColor::EditorBg: return "editor_bg";
        case ThemeColor::EditorFg: return "editor_fg";
        case ThemeColor::EditorCursor: return "caret";
        case ThemeColor::EditorSelection: return "selection";
        case ThemeColor::EditorCurrentLine: return "line_highlight";
        case ThemeColor::SyntaxKeyword: return "keyword";
        case ThemeColor::SyntaxString: return "string";
        case ThemeColor::SyntaxComment: return "comment";
        case ThemeColor::SyntaxNumber: return "number";
        case ThemeColor::SyntaxType: return "type";
        case ThemeColor::SyntaxFunction: return "function";
        case ThemeColor::SyntaxOperator: return "operator";
        case ThemeColor::GutterBg: return "gutter_bg";
        case ThemeColor::GutterText: return "gutter_text";
        case ThemeColor::GutterMarkerError: return "diag_error";
        case ThemeColor::GutterMarkerWarning: return "diag_warning";
        case ThemeColor::PanelBg: return "panel_bg";
        case ThemeColor::PanelBorder: return "panel_border";
        case ThemeColor::PanelHeader: return "panel_header";
        case ThemeColor::StatusBarBg: return "status_bg";
        case ThemeColor::TooltipBg: return "tooltip_bg";
        default: return "";
        }
    }

    std::vector<ThemeDefinition> themes_;
    std::vector<std::string> watchedDirs_;
    std::unordered_map<std::string, std::filesystem::file_time_type> fileTimes_;
    int current_ = -1;
};
