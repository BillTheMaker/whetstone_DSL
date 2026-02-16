#pragma once
// Step 345-347: Theme data model — headless semantic color definitions
// Does NOT depend on ImGui; provides color values that can be applied to ImGui or other renderers

#include <string>
#include <map>
#include <nlohmann/json.hpp>

struct Color4 {
    float r = 0, g = 0, b = 0, a = 1.0f;

    Color4() = default;
    Color4(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}

    // From hex "#RRGGBB" or "#RRGGBBAA"
    static Color4 fromHex(const std::string& hex) {
        Color4 c;
        if (hex.size() >= 7 && hex[0] == '#') {
            c.r = std::stoi(hex.substr(1, 2), nullptr, 16) / 255.0f;
            c.g = std::stoi(hex.substr(3, 2), nullptr, 16) / 255.0f;
            c.b = std::stoi(hex.substr(5, 2), nullptr, 16) / 255.0f;
            if (hex.size() >= 9) c.a = std::stoi(hex.substr(7, 2), nullptr, 16) / 255.0f;
        }
        return c;
    }

    std::string toHex() const {
        char buf[10];
        snprintf(buf, sizeof(buf), "#%02x%02x%02x",
                 (int)(r * 255), (int)(g * 255), (int)(b * 255));
        return buf;
    }

    bool isPureBlack() const { return r == 0 && g == 0 && b == 0; }
    bool isPureWhite() const { return r == 1 && g == 1 && b == 1; }
};

struct WhetstoneTheme {
    std::string name;

    // Backgrounds
    Color4 bg, bgAlt, bgPanel, bgPopup;

    // Text
    Color4 text, textDim, textAccent, textError, textWarning, textSuccess;

    // Borders
    Color4 border, borderFocused;

    // Accent (primary brand color)
    Color4 accent, accentHover, accentActive;

    // Selection
    Color4 selectionBg, selectionText;

    // Scrollbar
    Color4 scrollbar, scrollbarHover;

    // Diagnostics
    Color4 diagError, diagWarning, diagInfo, diagHint;

    // Syntax highlighting
    Color4 syntaxKeyword, syntaxString, syntaxNumber, syntaxComment;
    Color4 syntaxFunction, syntaxType, syntaxOperator, syntaxAnnotation;

    // Font sizes
    float fontSizePrimary = 14.0f;  // monospace code font
    float fontSizeUI = 13.0f;       // sans-serif UI font
    float fontSizeSmall = 11.0f;    // labels, badges

    nlohmann::json toJson() const {
        nlohmann::json j;
        j["name"] = name;
        // Backgrounds
        j["bg"] = bg.toHex(); j["bgAlt"] = bgAlt.toHex();
        j["bgPanel"] = bgPanel.toHex(); j["bgPopup"] = bgPopup.toHex();
        // Text
        j["text"] = text.toHex(); j["textDim"] = textDim.toHex();
        j["textAccent"] = textAccent.toHex();
        j["textError"] = textError.toHex(); j["textWarning"] = textWarning.toHex();
        j["textSuccess"] = textSuccess.toHex();
        // Borders
        j["border"] = border.toHex(); j["borderFocused"] = borderFocused.toHex();
        // Accent
        j["accent"] = accent.toHex(); j["accentHover"] = accentHover.toHex();
        j["accentActive"] = accentActive.toHex();
        // Selection
        j["selectionBg"] = selectionBg.toHex(); j["selectionText"] = selectionText.toHex();
        // Scrollbar
        j["scrollbar"] = scrollbar.toHex(); j["scrollbarHover"] = scrollbarHover.toHex();
        // Diagnostics
        j["diagError"] = diagError.toHex(); j["diagWarning"] = diagWarning.toHex();
        j["diagInfo"] = diagInfo.toHex(); j["diagHint"] = diagHint.toHex();
        // Syntax
        j["syntaxKeyword"] = syntaxKeyword.toHex(); j["syntaxString"] = syntaxString.toHex();
        j["syntaxNumber"] = syntaxNumber.toHex(); j["syntaxComment"] = syntaxComment.toHex();
        j["syntaxFunction"] = syntaxFunction.toHex(); j["syntaxType"] = syntaxType.toHex();
        j["syntaxOperator"] = syntaxOperator.toHex(); j["syntaxAnnotation"] = syntaxAnnotation.toHex();
        // Fonts
        j["fontSizePrimary"] = fontSizePrimary;
        j["fontSizeUI"] = fontSizeUI;
        j["fontSizeSmall"] = fontSizeSmall;
        return j;
    }

    static WhetstoneTheme fromJson(const nlohmann::json& j);
};

inline WhetstoneTheme getDefaultDarkTheme() {
    WhetstoneTheme t;
    t.name = "Whetstone Dark";

    // Background: deep blue-grey
    t.bg       = Color4::fromHex("#1a1d23");
    t.bgAlt    = Color4::fromHex("#22252b");
    t.bgPanel  = Color4::fromHex("#2a2d35");
    t.bgPopup  = Color4::fromHex("#30333b");

    // Text
    t.text        = Color4::fromHex("#e0e0e0");
    t.textDim     = Color4::fromHex("#808890");
    t.textAccent  = Color4::fromHex("#7aa2f7");
    t.textError   = Color4::fromHex("#f7768e");
    t.textWarning = Color4::fromHex("#e0af68");
    t.textSuccess = Color4::fromHex("#9ece6a");

    // Borders
    t.border        = Color4::fromHex("#3a3d45");
    t.borderFocused = Color4::fromHex("#7aa2f7");

    // Accent
    t.accent      = Color4::fromHex("#7aa2f7");
    t.accentHover = Color4::fromHex("#89b4fa");
    t.accentActive = Color4::fromHex("#5d8fea");

    // Selection
    t.selectionBg   = Color4::fromHex("#3d4f6f");
    t.selectionText = Color4::fromHex("#e0e0e0");

    // Scrollbar
    t.scrollbar      = Color4::fromHex("#3a3d45");
    t.scrollbarHover = Color4::fromHex("#4a4d55");

    // Diagnostics
    t.diagError   = Color4::fromHex("#f7768e");
    t.diagWarning = Color4::fromHex("#e0af68");
    t.diagInfo    = Color4::fromHex("#7dcfff");
    t.diagHint    = Color4::fromHex("#bb9af7");

    // Syntax
    t.syntaxKeyword    = Color4::fromHex("#7aa2f7");
    t.syntaxString     = Color4::fromHex("#9ece6a");
    t.syntaxNumber     = Color4::fromHex("#ff9e64");
    t.syntaxComment    = Color4::fromHex("#565f89");
    t.syntaxFunction   = Color4::fromHex("#7aa2f7");
    t.syntaxType       = Color4::fromHex("#2ac3de");
    t.syntaxOperator   = Color4::fromHex("#89ddff");
    t.syntaxAnnotation = Color4::fromHex("#bb9af7");

    return t;
}

inline WhetstoneTheme getDefaultLightTheme() {
    WhetstoneTheme t;
    t.name = "Whetstone Light";

    t.bg       = Color4::fromHex("#f0f0f0");
    t.bgAlt    = Color4::fromHex("#e8e8e8");
    t.bgPanel  = Color4::fromHex("#ffffff");
    t.bgPopup  = Color4::fromHex("#ffffff");

    t.text        = Color4::fromHex("#1a1a1a");
    t.textDim     = Color4::fromHex("#6a6a6a");
    t.textAccent  = Color4::fromHex("#2563eb");
    t.textError   = Color4::fromHex("#dc2626");
    t.textWarning = Color4::fromHex("#d97706");
    t.textSuccess = Color4::fromHex("#16a34a");

    t.border        = Color4::fromHex("#d0d0d0");
    t.borderFocused = Color4::fromHex("#2563eb");

    t.accent      = Color4::fromHex("#2563eb");
    t.accentHover = Color4::fromHex("#3b82f6");
    t.accentActive = Color4::fromHex("#1d4ed8");

    t.selectionBg   = Color4::fromHex("#bfdbfe");
    t.selectionText = Color4::fromHex("#1a1a1a");

    t.scrollbar      = Color4::fromHex("#c0c0c0");
    t.scrollbarHover = Color4::fromHex("#a0a0a0");

    t.diagError   = Color4::fromHex("#dc2626");
    t.diagWarning = Color4::fromHex("#d97706");
    t.diagInfo    = Color4::fromHex("#2563eb");
    t.diagHint    = Color4::fromHex("#7c3aed");

    t.syntaxKeyword    = Color4::fromHex("#2563eb");
    t.syntaxString     = Color4::fromHex("#16a34a");
    t.syntaxNumber     = Color4::fromHex("#d97706");
    t.syntaxComment    = Color4::fromHex("#6b7280");
    t.syntaxFunction   = Color4::fromHex("#7c3aed");
    t.syntaxType       = Color4::fromHex("#0891b2");
    t.syntaxOperator   = Color4::fromHex("#374151");
    t.syntaxAnnotation = Color4::fromHex("#7c3aed");

    return t;
}

inline WhetstoneTheme WhetstoneTheme::fromJson(const nlohmann::json& j) {
    WhetstoneTheme t;
    auto hex = [&](const std::string& key, Color4 fallback) -> Color4 {
        if (j.contains(key) && j[key].is_string()) return Color4::fromHex(j[key].get<std::string>());
        return fallback;
    };
    auto num = [&](const std::string& key, float fallback) -> float {
        if (j.contains(key) && j[key].is_number()) return j[key].get<float>();
        return fallback;
    };
    t.name = j.value("name", "Custom");
    auto def = getDefaultDarkTheme();
    t.bg = hex("bg", def.bg); t.bgAlt = hex("bgAlt", def.bgAlt);
    t.bgPanel = hex("bgPanel", def.bgPanel); t.bgPopup = hex("bgPopup", def.bgPopup);
    t.text = hex("text", def.text); t.textDim = hex("textDim", def.textDim);
    t.textAccent = hex("textAccent", def.textAccent);
    t.textError = hex("textError", def.textError); t.textWarning = hex("textWarning", def.textWarning);
    t.textSuccess = hex("textSuccess", def.textSuccess);
    t.border = hex("border", def.border); t.borderFocused = hex("borderFocused", def.borderFocused);
    t.accent = hex("accent", def.accent); t.accentHover = hex("accentHover", def.accentHover);
    t.accentActive = hex("accentActive", def.accentActive);
    t.selectionBg = hex("selectionBg", def.selectionBg); t.selectionText = hex("selectionText", def.selectionText);
    t.scrollbar = hex("scrollbar", def.scrollbar); t.scrollbarHover = hex("scrollbarHover", def.scrollbarHover);
    t.diagError = hex("diagError", def.diagError); t.diagWarning = hex("diagWarning", def.diagWarning);
    t.diagInfo = hex("diagInfo", def.diagInfo); t.diagHint = hex("diagHint", def.diagHint);
    t.syntaxKeyword = hex("syntaxKeyword", def.syntaxKeyword); t.syntaxString = hex("syntaxString", def.syntaxString);
    t.syntaxNumber = hex("syntaxNumber", def.syntaxNumber); t.syntaxComment = hex("syntaxComment", def.syntaxComment);
    t.syntaxFunction = hex("syntaxFunction", def.syntaxFunction); t.syntaxType = hex("syntaxType", def.syntaxType);
    t.syntaxOperator = hex("syntaxOperator", def.syntaxOperator); t.syntaxAnnotation = hex("syntaxAnnotation", def.syntaxAnnotation);
    t.fontSizePrimary = num("fontSizePrimary", def.fontSizePrimary);
    t.fontSizeUI = num("fontSizeUI", def.fontSizeUI);
    t.fontSizeSmall = num("fontSizeSmall", def.fontSizeSmall);
    return t;
}
