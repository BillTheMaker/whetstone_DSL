#pragma once
// Step 343: Icon system — Unicode symbols for panels, diagnostics, workflow, AST nodes
// No external icon font dependency — pure Unicode + text fallbacks

#include <string>
#include <map>

namespace WhetstoneIcons {

// File types
constexpr const char* FileSource   = "\xF0\x9F\x93\x84"; // document
constexpr const char* FileHeader   = "\xF0\x9F\x93\x83"; // page with curl
constexpr const char* FileConfig   = "\xE2\x9A\x99";     // gear
constexpr const char* FileImage    = "\xF0\x9F\x96\xBC"; // framed picture
constexpr const char* FileGeneric  = "\xF0\x9F\x93\x84"; // generic document
constexpr const char* Folder       = "\xF0\x9F\x93\x81"; // folder
constexpr const char* FolderOpen   = "\xF0\x9F\x93\x82"; // open folder

// Diagnostics
constexpr const char* DiagError    = "\xE2\x9B\x94";     // no entry
constexpr const char* DiagWarning  = "\xE2\x9A\xA0";     // warning
constexpr const char* DiagInfo     = "\xE2\x84\xB9";     // info
constexpr const char* DiagHint     = "\xF0\x9F\x92\xA1"; // light bulb

// Workflow status
constexpr const char* Pending      = "\xE2\x8F\xB3";     // hourglass
constexpr const char* InProgress   = "\xE2\x96\xB6";     // play
constexpr const char* Complete     = "\xE2\x9C\x85";     // check mark
constexpr const char* Rejected     = "\xE2\x9D\x8C";     // cross mark
constexpr const char* Review       = "\xF0\x9F\x91\x81"; // eye

// AST nodes
constexpr const char* NodeFunction = "\xC6\x92";         // function ƒ
constexpr const char* NodeClass    = "\xE2\x97\x86";     // diamond
constexpr const char* NodeVariable = "\xF0\x9D\x91\xA5"; // math x
constexpr const char* NodeModule   = "\xE2\x96\xA3";     // square
constexpr const char* NodeAnnotation = "\xF0\x9F\x8F\xB7"; // label

// Navigation
constexpr const char* ArrowRight   = "\xE2\x96\xB6";
constexpr const char* ArrowDown    = "\xE2\x96\xBC";
constexpr const char* Search       = "\xF0\x9F\x94\x8D";
constexpr const char* Settings     = "\xE2\x9A\x99";

// Text fallbacks (ASCII-safe)
inline std::string fallback(const std::string& icon) {
    static std::map<std::string, std::string> fallbacks = {
        {DiagError, "[ERR]"}, {DiagWarning, "[WARN]"},
        {DiagInfo, "[INFO]"}, {DiagHint, "[HINT]"},
        {Pending, "[...]"}, {InProgress, "[>>]"},
        {Complete, "[OK]"}, {Rejected, "[FAIL]"}, {Review, "[?]"},
        {NodeFunction, "fn"}, {NodeClass, "cls"},
        {NodeVariable, "var"}, {NodeModule, "mod"},
        {FileSource, "[]"}, {Folder, "/"}, {FolderOpen, "/"},
    };
    auto it = fallbacks.find(icon);
    return it != fallbacks.end() ? it->second : icon;
}

// Get icon for AST node type
inline std::string iconForNodeType(const std::string& conceptType) {
    if (conceptType == "Function" || conceptType == "AsyncFunction" ||
        conceptType == "MethodDeclaration") return NodeFunction;
    if (conceptType == "ClassDeclaration" || conceptType == "InterfaceDeclaration") return NodeClass;
    if (conceptType == "Variable") return NodeVariable;
    if (conceptType == "Module") return NodeModule;
    if (conceptType.find("Annotation") != std::string::npos) return NodeAnnotation;
    if (conceptType == "EnumDeclaration") return NodeClass;
    if (conceptType == "NamespaceDeclaration") return Folder;
    return NodeModule; // default
}

// Get icon for diagnostic severity (by int: 1=error, 2=warning, 3=info, 4=hint)
inline std::string iconForSeverity(int severity) {
    switch (severity) {
        case 1: return DiagError;
        case 2: return DiagWarning;
        case 3: return DiagInfo;
        case 4: return DiagHint;
        default: return DiagInfo;
    }
}

// Get icon for diagnostic severity (by name)
inline std::string iconForSeverity(const std::string& severity) {
    if (severity == "error") return DiagError;
    if (severity == "warning") return DiagWarning;
    if (severity == "info") return DiagInfo;
    if (severity == "hint") return DiagHint;
    return DiagInfo;
}

// Get icon for workflow status
inline std::string iconForWorkflowStatus(const std::string& status) {
    if (status == "pending" || status == "ready") return Pending;
    if (status == "assigned" || status == "in-progress") return InProgress;
    if (status == "complete") return Complete;
    if (status == "rejected") return Rejected;
    if (status == "review") return Review;
    return Pending;
}

} // namespace WhetstoneIcons
